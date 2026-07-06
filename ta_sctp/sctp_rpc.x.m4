/* SPDX-License-Identifier: MIT */
/* Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved. */
/** @file
 * @brief RPC for SCTP client operations
 *
 * The RPCs of rpcs_sctp, a thin layer over ta_sctp, which drives SCTP
 * from the RPC server process over the lksctp sockets API. Add this
 * file to the rpcxdr definitions of the engine platform and of the
 * agent platform:
 *
 *   TE_LIB_PARMS([rpcxdr], [<platform>], [],
 *                [--with-rpcdefs=tarpc_job.x.m4,../ta_sctp/sctp_rpc.x.m4])
 *
 * No handle survives between calls: each call opens an association and
 * closes it. Results come back as newline/tab record text - the engine
 * side parses them.
 */

/* sctp_probe(): open an association and report what it negotiated. */
struct tarpc_sctp_probe_in {
    struct tarpc_in_arg common;

    string          host<>;
    tarpc_int       port;
    tarpc_int       timeout_ms;
};

struct tarpc_sctp_probe_out {
    struct tarpc_out_arg common;

    tarpc_int       retval;
    string          result<>;
};

/* sctp_echo(): send one message, receive one, and close. */
struct tarpc_sctp_echo_in {
    struct tarpc_in_arg common;

    string          host<>;
    tarpc_int       port;
    string          payload<>;
    tarpc_int       timeout_ms;
};

struct tarpc_sctp_echo_out {
    struct tarpc_out_arg common;

    tarpc_int       retval;
    string          reply<>;
};

program sctp
{
    version ver0
    {
        RPC_DEF(sctp_probe)
        RPC_DEF(sctp_echo)
    } = 1;
} = 38;
