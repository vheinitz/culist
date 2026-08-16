# culist — Comprehensive, Usable LIS Tool 🔬

[![License: Apache-2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![C++](https://img.shields.io/badge/C%2B%2B-blue.svg)](#)

**CULIST [coolest]** — a *C*omprehensive, *u*sable **LIS tool** for connecting
devices and systems to a **Laboratory Information System (LIS)** over the
**ASTM E1394 (LIS01-A2 / LIS2-A2)** protocol.

## Motivation

> *"As I had to connect a customer's device to an LIS I found out there are no
> really usable tools for analysing and debugging ASTM E1394 (LIS01-A2, LIS2-A2)
> protocols. Some commercial tools I got for evaluation were old, bad,
> vendor-specific, expensive — or (mostly) all of these together."*

CULIST fills that gap for the developer sitting between an instrument and a LIS.

## Features

- **Client/server TCP connection mode** — act as either end of the line.
- **Parsing ASTM trace files** into a tree-like structure.
- **Display each field separately** — inspect every record/field of a message.
- Supports **LIS01-A2** and **LIS2-A2** framing.

## Roadmap / planned

- Serial (RS-232) connection
- HL7 support
- Monitoring files/directories for send/receive actions
- Connection test (ENQ–ACK–EOT), configurable timing & error simulation
- DB connection

See [`doc/readme.txt`](doc/readme.txt) for the full TODO list.

## Project layout

```
src/astm.{h,cpp}       ASTM E1394 record model + control chars (ENQ/ACK/EOT/…)
src/culistgui.{h,cpp}  main GUI
src/persistence.{h,cpp}  project/state persistence
CULIST.pro             qmake project
```

## Requirements

- Qt (Gui), a C++ compiler

## Build

```bash
qmake && make
```

## Related

This is the most complete LIS tool in the family — see also
[GoodnightAstm](../GoodnightAstm) (parser), [ASTMViewer](../ASTMViewer) (message
tree viewer) and [cl2](../cl2) (reusable `lis/` module).

## License

[Apache License 2.0](LICENSE) © Valentin Heinitz
