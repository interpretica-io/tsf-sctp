/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief SCTP TAPI: RPC client wrappers
 *
 * The rcf_rpc_call() boilerplate behind tapi_sctp. The RPCs return
 * te_errno; an RPC transport failure is mapped to TE_ECORRUPTED.
 */

#define TE_LGR_USER     "TAPI SCTP RPC"

#include "te_config.h"

#include <string.h>

#include "te_defs.h"
#include "te_errno.h"
#include "te_string.h"
#include "logger_api.h"
#include "tapi_rpc_internal.h"
#include "tarpc.h"

#include "tapi_sctp_rpc.h"

#define CHECK_RPC_ERRNO_UNCHANGED(_func, _var) \
    CHECK_RETVAL_VAR_ERR_COND(_func, _var, false,                    \
                              TE_RC(TE_TAPI, TE_ECORRUPTED), false)

static void
take_string(te_string *dst, const char *src)
{
    if (dst != NULL && src != NULL)
        te_string_append(dst, "%s", src);
}

/* See description in tapi_sctp_rpc.h */
te_errno
rpc_sctp_probe(rcf_rpc_server *rpcs, const char *host, int port,
               int timeout_ms, te_string *result)
{
    tarpc_sctp_probe_in in;
    tarpc_sctp_probe_out out;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.host = (char *)(host != NULL ? host : "");
    in.port = port;
    in.timeout_ms = timeout_ms;

    rcf_rpc_call(rpcs, "sctp_probe", &in, &out);
    CHECK_RPC_ERRNO_UNCHANGED(sctp_probe, out.retval);
    TAPI_RPC_LOG(rpcs, sctp_probe, "%s:%d", "%r",
                 host != NULL ? host : "", port, out.retval);

    if (out.retval == 0)
        take_string(result, out.result);
    RETVAL_TE_ERRNO(sctp_probe, out.retval);
}

/* See description in tapi_sctp_rpc.h */
te_errno
rpc_sctp_echo(rcf_rpc_server *rpcs, const char *host, int port,
              const char *payload, int timeout_ms, te_string *reply)
{
    tarpc_sctp_echo_in in;
    tarpc_sctp_echo_out out;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.host = (char *)(host != NULL ? host : "");
    in.port = port;
    in.payload = (char *)(payload != NULL ? payload : "");
    in.timeout_ms = timeout_ms;

    rcf_rpc_call(rpcs, "sctp_echo", &in, &out);
    CHECK_RPC_ERRNO_UNCHANGED(sctp_echo, out.retval);
    TAPI_RPC_LOG(rpcs, sctp_echo, "%s:%d", "%r",
                 host != NULL ? host : "", port, out.retval);

    if (out.retval == 0)
        take_string(reply, out.reply);
    RETVAL_TE_ERRNO(sctp_echo, out.retval);
}
