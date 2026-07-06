/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief Agent-side SCTP client over the Linux lksctp sockets API
 *
 * Written against lksctp-tools 1.0.19 (@c <netinet/sctp.h> plus the
 * kernel's @c <linux/sctp.h>): a one-to-one SCTP association, read-only
 * except for an optional single-message echo. The association is opened
 * with a non-blocking connect bounded by a timeout, so an unreachable
 * peer does not hang the RPC server.
 */

#define TE_LGR_USER     "TA SCTP"

#include "te_config.h"

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/sctp.h>

#include "te_defs.h"
#include "te_errno.h"
#include "te_string.h"
#include "te_str.h"
#include "logger_api.h"

#include "ta_sctp.h"

/** Default connect/IO timeout when the caller passes 0, ms. */
#define TA_SCTP_DEFAULT_TIMEOUT_MS 5000

/** One sockaddr to a numeric address string. */
static void
sctp_addr_str(const struct sockaddr *sa, char *buf, size_t len)
{
    if (getnameinfo(sa, sa->sa_family == AF_INET6 ?
                    sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in),
                    buf, len, NULL, 0, NI_NUMERICHOST) != 0)
    {
        te_strlcpy(buf, "?", len);
    }
}

/**
 * Open a one-to-one SCTP association to @p host : @p port, bounded by
 * @p timeout_ms. On success @p *out_fd is the connected socket.
 */
static te_errno
sctp_connect(const char *host, int port, int timeout_ms, int *out_fd)
{
    struct addrinfo hints;
    struct addrinfo *res = NULL;
    struct addrinfo *ai;
    char portbuf[16];
    te_errno rc = TE_RC(TE_TA_UNIX, TE_ECONNREFUSED);

    if (timeout_ms <= 0)
        timeout_ms = TA_SCTP_DEFAULT_TIMEOUT_MS;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_SCTP;
    TE_SPRINTF(portbuf, "%d", port);

    if (getaddrinfo(host, portbuf, &hints, &res) != 0)
    {
        ERROR("Cannot resolve %s:%d", host, port);
        return TE_RC(TE_TA_UNIX, TE_ENOENT);
    }

    for (ai = res; ai != NULL; ai = ai->ai_next)
    {
        int fd = socket(ai->ai_family, SOCK_STREAM, IPPROTO_SCTP);
        int flags;
        fd_set wset;
        struct timeval tv;
        int err = 0;
        socklen_t errlen = sizeof(err);
        struct timeval io;

        if (fd < 0)
        {
            rc = TE_OS_RC(TE_TA_UNIX, errno);
            continue;
        }

        flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        if (connect(fd, ai->ai_addr, ai->ai_addrlen) == 0)
            goto connected;
        if (errno != EINPROGRESS)
        {
            rc = TE_OS_RC(TE_TA_UNIX, errno);
            close(fd);
            continue;
        }

        FD_ZERO(&wset);
        FD_SET(fd, &wset);
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        if (select(fd + 1, NULL, &wset, NULL, &tv) <= 0)
        {
            rc = TE_RC(TE_TA_UNIX, TE_ETIMEDOUT);
            close(fd);
            continue;
        }
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &errlen) != 0 ||
            err != 0)
        {
            rc = TE_OS_RC(TE_TA_UNIX, err != 0 ? err : errno);
            close(fd);
            continue;
        }

connected:
        fcntl(fd, F_SETFL, flags);
        io.tv_sec = timeout_ms / 1000;
        io.tv_usec = (timeout_ms % 1000) * 1000;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &io, sizeof(io));
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &io, sizeof(io));
        *out_fd = fd;
        freeaddrinfo(res);
        return 0;
    }

    freeaddrinfo(res);
    return rc;
}

/** Append the local/peer address list of @p fd as "tag\t<addr>" lines. */
static void
sctp_dump_addrs(int fd, te_string *result)
{
    struct sockaddr *addrs = NULL;
    int n;
    int i;
    const struct sockaddr *sa;
    char buf[128];

    n = sctp_getladdrs(fd, 0, &addrs);
    sa = addrs;
    for (i = 0; i < n && sa != NULL; i++)
    {
        sctp_addr_str(sa, buf, sizeof(buf));
        te_string_append(result, "laddr\t%s\n", buf);
        sa = (const struct sockaddr *)((const char *)sa +
             (sa->sa_family == AF_INET6 ? sizeof(struct sockaddr_in6) :
              sizeof(struct sockaddr_in)));
    }
    if (n > 0)
        sctp_freeladdrs(addrs);

    addrs = NULL;
    n = sctp_getpaddrs(fd, 0, &addrs);
    sa = addrs;
    for (i = 0; i < n && sa != NULL; i++)
    {
        sctp_addr_str(sa, buf, sizeof(buf));
        te_string_append(result, "paddr\t%s\n", buf);
        sa = (const struct sockaddr *)((const char *)sa +
             (sa->sa_family == AF_INET6 ? sizeof(struct sockaddr_in6) :
              sizeof(struct sockaddr_in)));
    }
    if (n > 0)
        sctp_freepaddrs(addrs);
}

/* See description in ta_sctp.h */
te_errno
ta_sctp_probe(const char *host, int port, int timeout_ms, te_string *result)
{
    int fd = -1;
    struct sctp_status status;
    socklen_t len = sizeof(status);
    te_errno rc;

    rc = sctp_connect(host, port, timeout_ms, &fd);
    if (rc != 0)
        return rc;

    memset(&status, 0, sizeof(status));
    if (sctp_opt_info(fd, 0, SCTP_STATUS, &status, &len) != 0)
    {
        rc = TE_OS_RC(TE_TA_UNIX, errno);
        close(fd);
        return rc;
    }

    te_string_append(result, "status\t%d\t%u\t%u\t%u\n",
                     (int)status.sstat_state,
                     (unsigned)status.sstat_instrms,
                     (unsigned)status.sstat_outstrms,
                     (unsigned)status.sstat_primary.spinfo_mtu);
    sctp_dump_addrs(fd, result);

    close(fd);
    return 0;
}

/* See description in ta_sctp.h */
te_errno
ta_sctp_echo(const char *host, int port, const char *payload, int timeout_ms,
             te_string *reply)
{
    int fd = -1;
    char buf[4096];
    int n;
    struct sctp_sndrcvinfo sri;
    int msg_flags = 0;
    te_errno rc;

    rc = sctp_connect(host, port, timeout_ms, &fd);
    if (rc != 0)
        return rc;

    if (sctp_sendmsg(fd, payload, payload != NULL ? strlen(payload) : 0,
                     NULL, 0, 0, 0, 0, 0, 0) < 0)
    {
        rc = TE_OS_RC(TE_TA_UNIX, errno);
        close(fd);
        return rc;
    }

    memset(&sri, 0, sizeof(sri));
    n = sctp_recvmsg(fd, buf, sizeof(buf) - 1, NULL, NULL, &sri, &msg_flags);
    if (n < 0)
    {
        rc = TE_OS_RC(TE_TA_UNIX, errno);
        close(fd);
        return rc;
    }

    buf[n] = '\0';
    te_string_append(reply, "%s", buf);

    close(fd);
    return 0;
}
