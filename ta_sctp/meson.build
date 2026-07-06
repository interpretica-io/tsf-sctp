# SPDX-License-Identifier: MIT
# Copyright (C) 2026 Interpretica, Unipessoal Lda. All rights reserved.
#
# External TE library (see TE_EXT_REPO): agent-side SCTP client over the
# Linux lksctp sockets API. The agent and its RPC server both link it.

headers += files(
    'ta_sctp.h',
)

sources += files(
    'ta_sctp.c',
)

te_libs += [
    'logger_core',
    'tools',
]

cc = meson.get_compiler('c')
sctp_dep = dependency('libsctp', required: false)
if not sctp_dep.found()
    sctp_dep = cc.find_library('sctp', has_headers: 'netinet/sctp.h')
endif
deps += [ sctp_dep ]
