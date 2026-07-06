/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief SCTP TAPI: RPC client wrappers
 *
 * Client wrappers of the sctp_* RPCs, see sctp_rpc.x.m4. Tests use
 * tapi_sctp.h; these are the calls behind it, one per RPC.
 */

#ifndef __TAPI_SCTP_RPC_H__
#define __TAPI_SCTP_RPC_H__

#include "te_errno.h"
#include "te_string.h"
#include "rcf_rpc.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Open an association and read its status/addresses (raw record text). */
extern te_errno rpc_sctp_probe(rcf_rpc_server *rpcs, const char *host,
                               int port, int timeout_ms, te_string *result);

/** Send one message, receive one; @p reply is the bytes received. */
extern te_errno rpc_sctp_echo(rcf_rpc_server *rpcs, const char *host,
                              int port, const char *payload, int timeout_ms,
                              te_string *reply);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* !__TAPI_SCTP_RPC_H__ */
