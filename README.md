# tsf-sctp

Driving SCTP (RFC 9260) from a Test Agent, packaged as an external Test
Environment (TE) repository (consumed with the `TE_EXT_REPO` builder
directive). It opens associations over a low-level C sockets API — the
Linux **lksctp** stack, no program spawned.

Three libraries:

- `ta_sctp` — agent side. A one-to-one SCTP client over
  `<netinet/sctp.h>` (`-lsctp`): open an association to a peer, read
  what it negotiated (state, inbound/outbound streams, path MTU, the
  multihoming local/peer address lists via `sctp_getladdrs`/
  `sctp_getpaddrs`), and optionally echo one message. Read/connect only.
- `rpcs_sctp` — the `sctp_*` RPCs for the agent's RPC server.
- `tapi_sctp` — engine side: `tapi_sctp_probe()` into a
  `tapi_sctp_assoc`, `tapi_sctp_echo()`, and `tapi_sctp_established()`.

TE's traffic stack carries TCP/UDP/ICMP but not SCTP.

## Why SCTP

SCTP is the transport under the telecom signalling stacks — **SIGTRAN**
(M3UA/M2UA/SUA), **Diameter**, **S1AP/NGAP/X2AP** (LTE/5G control
plane), **PFCP** — and WebRTC data channels. None of these can be
exercised over TCP/UDP. SCTP also brings **multihoming** (one
association over several paths, with failover) and **multi-streaming**
(no head-of-line blocking), both of which are behaviour worth testing
and neither of which TCP offers. `tsf-sctp` is the transport other
telecom modules (a future `tsf-diameter`, `tsf-ngap`) would build on,
the way `tsf-can` opened the automotive/industrial bus.

## Agent host requirements

- **lksctp-tools** development files — Debian/Ubuntu:
  `apt install libsctp-dev` (gives `<netinet/sctp.h>` and `-lsctp`,
  pkg-config `libsctp`).
- An SCTP-capable kernel (the stock Linux `sctp` module); `tsf-sctp`
  loads nothing and only connects out.

## Usage

Declare the repository in an external libraries catalog:

```yaml
repositories:
  - name: tsf_sctp
    url: https://github.com/interpretica-io/tsf-sctp.git
    ref: <tag>
    libs:
      - ta_sctp
      - rpcs_sctp
      - tapi_sctp
```

In `builder.conf`:

```
TE_EXT_REPO_USE([tsf_sctp], [], [ta_sctp rpcs_sctp tapi_sctp])
TE_LIB_PARMS([rpcxdr], [${TE_HOST}], [],
             [--with-rpcdefs=tarpc_job.x.m4,../ta_sctp/sctp_rpc.x.m4])
```

and list `ta_sctp rpcs_sctp` among the RPC server's libraries, passing
`--extra-deps='libpcre2-8 libsctp'` to its `ta_rpcprovider` build. The
RPC program number is **38**.

## What was verified, and what was not

The lksctp sockets usage (`socket(…, IPPROTO_SCTP)`, non-blocking
connect, `sctp_opt_info`/`SCTP_STATUS`, `sctp_getladdrs`/`getpaddrs`,
`sctp_sendmsg`/`sctp_recvmsg`) was **written against the real
lksctp-tools 1.0.19 headers** (`<netinet/sctp.h>` + `<linux/sctp.h>`) —
the `struct sctp_status` field names and function prototypes were read
from those headers. It is **Linux-only**, so it was not compiled on the
authoring (macOS) host; verification is the native build and run of the
companion `tsf-sctp-ts` suite on a Linux agent.
