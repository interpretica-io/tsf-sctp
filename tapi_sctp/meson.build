# SPDX-License-Identifier: MIT
# Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved.
#
# External TE library (see TE_EXT_REPO): engine-side TAPI for SCTP on
# agents. Built as a shared library on the engine platform. It needs
# the sctp_* RPCs in rpcxdr, so add ../ta_sctp/sctp_rpc.x.m4 to the
# rpcxdr definitions of the engine platform.

if get_option('engine')
    build_lib_shared = true
endif
build_lib_static = not get_option('engine')
install_lib = install_dev

headers += files(
    'tapi_sctp.h',
    'tapi_sctp_rpc.h',
)

sources += files(
    'tapi_sctp.c',
    'tapi_sctp_rpc.c',
)

te_libs += [
    'tapi',
    'tapi_rpc',
    'rcfrpc',
    'confapi',
    'rcfapi',
    'rpcxdr',
    'logger_core',
    'tools',
]
