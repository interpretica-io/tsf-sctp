/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief Agent-side SCTP client
 *
 * An SCTP (RFC 9260) client on top of the Linux lksctp sockets API
 * (@c <netinet/sctp.h>, @c -lsctp): the library is linked into the
 * agent and called in-process, nothing is spawned. The agent and its
 * RPC server both link this; the RPCs (see sctp_rpc.x.m4) are thin
 * wrappers over these functions.
 *
 * Read/connect only: it opens a one-to-one SCTP association to a peer,
 * reads what was negotiated (state, streams, MTU, the multihoming
 * address lists), optionally exchanges one message, and closes. It
 * neither listens nor changes anything on the peer.
 *
 * Results come back as newline-separated text, one record per line with
 * tab-separated fields, the same shape tsf-usb uses - the engine side
 * parses them.
 */

#ifndef __TA_SCTP_H__
#define __TA_SCTP_H__

#include "te_errno.h"
#include "te_string.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Open an SCTP association to @p host : @p port and read what it
 * negotiated.
 *
 * Lines, tab-separated:
 * @c "status\\tstate\\tin_streams\\tout_streams\\tmtu"; then one
 * @c "laddr\\t<addr>" per local (multihoming) address and one
 * @c "paddr\\t<addr>" per peer address. @a state is the numeric
 * @c sctp_sstat_state (e.g. 4 = ESTABLISHED).
 *
 * @param[in]  host         Peer host (name or address).
 * @param[in]  port         Peer port.
 * @param[in]  timeout_ms   Connect/IO timeout, ms (0 for a default).
 * @param[out] result       The association records.
 *
 * @return Status code.
 * @retval TE_ETIMEDOUT     The 4-way handshake did not complete in time.
 * @retval TE_ECONNREFUSED  The peer refused (ABORT) or is unreachable.
 */
extern te_errno ta_sctp_probe(const char *host, int port, int timeout_ms,
                              te_string *result);

/**
 * Open an association, send one message, receive one, and close.
 *
 * @param[in]  host         Peer host.
 * @param[in]  port         Peer port.
 * @param[in]  payload      Bytes to send (a NUL-terminated string).
 * @param[in]  timeout_ms   Connect/IO timeout, ms (0 for a default).
 * @param[out] reply        The bytes received back (appended as text).
 *
 * @return Status code.
 */
extern te_errno ta_sctp_echo(const char *host, int port, const char *payload,
                             int timeout_ms, te_string *reply);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !__TA_SCTP_H__ */
