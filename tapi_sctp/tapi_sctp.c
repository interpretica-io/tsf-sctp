/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief Driving SCTP from a test
 *
 * The engine-neutral layer over the sctp_* RPCs: it asks the agent to
 * open an association and parses the newline/tab status + address
 * records into a #tapi_sctp_assoc.
 */

#define TE_LGR_USER     "TAPI SCTP"

#include "te_config.h"

#include <stdlib.h>
#include <string.h>

#include "te_defs.h"
#include "te_errno.h"
#include "te_alloc.h"
#include "te_str.h"
#include "te_string.h"
#include "te_vector.h"
#include "logger_api.h"

#include "tapi_sctp.h"
#include "tapi_sctp_rpc.h"

static void
sctp_assoc_init(tapi_sctp_assoc *assoc)
{
    memset(assoc, 0, sizeof(*assoc));
    assoc->local_addrs = (te_vec)TE_VEC_INIT(char *);
    assoc->peer_addrs = (te_vec)TE_VEC_INIT(char *);
}

/** Parse the newline/tab records from sctp_probe into @p assoc. */
static void
sctp_parse(const char *text, tapi_sctp_assoc *assoc)
{
    const char *line = text;

    while (line != NULL && *line != '\0')
    {
        const char *nl = strchr(line, '\n');
        size_t len = nl != NULL ? (size_t)(nl - line) : strlen(line);
        char *one = TE_STRNDUP(line, len);

        if (strncmp(one, "status\t", 7) == 0)
        {
            int st = 0;
            unsigned in = 0;
            unsigned out = 0;
            unsigned mtu = 0;

            if (sscanf(one, "status\t%d\t%u\t%u\t%u", &st, &in, &out,
                       &mtu) >= 1)
            {
                assoc->state = st;
                assoc->in_streams = in;
                assoc->out_streams = out;
                assoc->mtu = mtu;
            }
        }
        else if (strncmp(one, "laddr\t", 6) == 0)
        {
            char *a = TE_STRDUP(one + 6);

            TE_VEC_APPEND(&assoc->local_addrs, a);
        }
        else if (strncmp(one, "paddr\t", 6) == 0)
        {
            char *a = TE_STRDUP(one + 6);

            TE_VEC_APPEND(&assoc->peer_addrs, a);
        }

        free(one);
        line = nl != NULL ? nl + 1 : NULL;
    }
}

/* See description in tapi_sctp.h */
te_errno
tapi_sctp_probe(rcf_rpc_server *rpcs, const char *host, int port,
                int timeout_ms, tapi_sctp_assoc *assoc)
{
    te_string raw = TE_STRING_INIT;
    te_errno rc;

    sctp_assoc_init(assoc);

    rc = rpc_sctp_probe(rpcs, host, port, timeout_ms, &raw);
    if (rc == 0)
        sctp_parse(te_string_value(&raw), assoc);

    te_string_free(&raw);
    return rc;
}

/* See description in tapi_sctp.h */
te_errno
tapi_sctp_echo(rcf_rpc_server *rpcs, const char *host, int port,
               const char *payload, int timeout_ms, te_string *reply)
{
    return rpc_sctp_echo(rpcs, host, port, payload, timeout_ms, reply);
}

/* See description in tapi_sctp.h */
bool
tapi_sctp_established(const tapi_sctp_assoc *assoc)
{
    return assoc->state == TAPI_SCTP_STATE_ESTABLISHED;
}

/* See description in tapi_sctp.h */
void
tapi_sctp_assoc_log(const tapi_sctp_assoc *assoc)
{
    te_string s = TE_STRING_INIT;
    char * const *a;

    te_string_append(&s, "SCTP assoc: state %d, %u in / %u out streams, "
                     "mtu %u", assoc->state, assoc->in_streams,
                     assoc->out_streams, assoc->mtu);
    te_string_append(&s, "\n  local:");
    TE_VEC_FOREACH((te_vec *)&assoc->local_addrs, a)
        te_string_append(&s, " %s", *a);
    te_string_append(&s, "\n  peer:");
    TE_VEC_FOREACH((te_vec *)&assoc->peer_addrs, a)
        te_string_append(&s, " %s", *a);

    RING("%s", te_string_value(&s));
    te_string_free(&s);
}

/* See description in tapi_sctp.h */
void
tapi_sctp_assoc_free(tapi_sctp_assoc *assoc)
{
    char **a;

    TE_VEC_FOREACH(&assoc->local_addrs, a)
        free(*a);
    te_vec_free(&assoc->local_addrs);
    TE_VEC_FOREACH(&assoc->peer_addrs, a)
        free(*a);
    te_vec_free(&assoc->peer_addrs);
    memset(assoc, 0, sizeof(*assoc));
}
