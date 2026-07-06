/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief Driving SCTP from a test
 *
 * @defgroup tapi_sctp SCTP (tapi_sctp)
 * @{
 *
 * Opening an SCTP (RFC 9260) association to a peer from a Test Agent
 * and reading what it negotiated: the association state, the inbound
 * and outbound stream counts, the path MTU, and the multihoming
 * address lists. SCTP is the transport under the telecom signalling
 * stacks (SIGTRAN/M3UA, Diameter, S1AP/NGAP) and WebRTC data channels,
 * none of which TE's TCP/UDP stack can carry.
 *
 * The association runs in the agent's RPC server process over the Linux
 * lksctp sockets API. It is read-only (connect, read status, optionally
 * echo one message, close) - it does not listen or change the peer.
 *
 * @code
 * tapi_sctp_assoc assoc;
 *
 * CHECK_RC(tapi_sctp_probe(rpcs, "10.0.0.1", 36412, 5000, &assoc));
 * if (!tapi_sctp_established(&assoc))
 *     TEST_VERDICT("no SCTP association to the peer");
 * RING("%u in / %u out streams, %u peer path(s)", assoc.in_streams,
 *      assoc.out_streams, (unsigned)te_vec_size(&assoc.peer_addrs));
 * tapi_sctp_assoc_free(&assoc);
 * @endcode
 */

#ifndef __TAPI_SCTP_H__
#define __TAPI_SCTP_H__

#include "te_defs.h"
#include "te_errno.h"
#include "te_string.h"
#include "te_vector.h"
#include "rcf_rpc.h"

#ifdef __cplusplus
extern "C" {
#endif

/** A negotiated SCTP association, as seen from the agent. */
typedef struct tapi_sctp_assoc {
    /** @c sctp_sstat_state (4 = ESTABLISHED; see enum sctp_sstat_state). */
    int state;
    /** Inbound streams negotiated. */
    unsigned int in_streams;
    /** Outbound streams negotiated. */
    unsigned int out_streams;
    /** Path MTU of the primary path. */
    unsigned int mtu;
    /** Vector of @c char* : the local (multihoming) addresses. */
    te_vec local_addrs;
    /** Vector of @c char* : the peer addresses. */
    te_vec peer_addrs;
} tapi_sctp_assoc;

/** SCTP ESTABLISHED state (@c sctp_sstat_state). */
#define TAPI_SCTP_STATE_ESTABLISHED 4

/**
 * Open an association to @p host : @p port and read it.
 *
 * @param[in]  rpcs         RPC server on the agent.
 * @param[in]  host         Peer host (name or address).
 * @param[in]  port         Peer port.
 * @param[in]  timeout_ms   Connect/IO timeout, ms (0 for a default).
 * @param[out] assoc        The association; release with
 *                          tapi_sctp_assoc_free().
 *
 * @return Status code.
 * @retval TE_ETIMEDOUT     No association formed in time.
 * @retval TE_ECONNREFUSED  The peer refused or is unreachable.
 */
extern te_errno tapi_sctp_probe(rcf_rpc_server *rpcs, const char *host,
                                int port, int timeout_ms,
                                tapi_sctp_assoc *assoc);

/**
 * Open an association, send one message, receive one, and close.
 *
 * @param[in]  rpcs         RPC server on the agent.
 * @param[in]  host         Peer host.
 * @param[in]  port         Peer port.
 * @param[in]  payload      Message to send.
 * @param[in]  timeout_ms   Timeout, ms (0 for a default).
 * @param[out] reply        The bytes received back.
 *
 * @return Status code.
 */
extern te_errno tapi_sctp_echo(rcf_rpc_server *rpcs, const char *host,
                               int port, const char *payload, int timeout_ms,
                               te_string *reply);

/**
 * Is the association established?
 *
 * @param assoc         Association.
 *
 * @return @c true when @a state is ESTABLISHED.
 */
extern bool tapi_sctp_established(const tapi_sctp_assoc *assoc);

/**
 * Write an association into the log.
 *
 * @param assoc         Association.
 */
extern void tapi_sctp_assoc_log(const tapi_sctp_assoc *assoc);

/**
 * Release an association.
 *
 * @param assoc         Association.
 */
extern void tapi_sctp_assoc_free(tapi_sctp_assoc *assoc);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !__TAPI_SCTP_H__ */

/**@} <!-- END tapi_sctp --> */
