/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief SCTP RPC server library
 *
 * The sctp_* RPCs (see sctp_rpc.x.m4) on top of ta_sctp.
 * TARPC_FUNC_STATIC() binds an RPC to the function of the same name,
 * so each RPC has a plain C function first and the wrapper after it.
 */

#define TE_LGR_USER     "RPC SCTP"

#include "te_config.h"

#include <string.h>

#include "te_defs.h"
#include "te_errno.h"
#include "te_alloc.h"
#include "te_str.h"
#include "te_string.h"
#include "rpc_server.h"

#include "ta_sctp.h"

/* Hand a te_string result over to an RPC string field (never NULL). */
static char *
take(te_string *str)
{
    return str->ptr != NULL ? str->ptr : TE_STRDUP("");
}

static te_errno
sctp_probe(const char *host, int port, int timeout_ms, char **result)
{
    te_string r = TE_STRING_INIT;
    te_errno rc = ta_sctp_probe(host, port, timeout_ms, &r);

    *result = take(&r);
    return rc;
}

TARPC_FUNC_STATIC(sctp_probe, {},
{
    MAKE_CALL(out->retval = func(in->host, in->port, in->timeout_ms,
                                 &out->result));
    out->common.errno_changed = false;
})

static te_errno
sctp_echo(const char *host, int port, const char *payload, int timeout_ms,
          char **reply)
{
    te_string r = TE_STRING_INIT;
    te_errno rc = ta_sctp_echo(host, port, payload, timeout_ms, &r);

    *reply = take(&r);
    return rc;
}

TARPC_FUNC_STATIC(sctp_echo, {},
{
    MAKE_CALL(out->retval = func(in->host, in->port, in->payload,
                                 in->timeout_ms, &out->reply));
    out->common.errno_changed = false;
})
