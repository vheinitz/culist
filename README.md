# culist

[![License: Apache-2.0](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)

Qt desktop tool for testing ASTM connections between laboratory instruments
and a LIS: send and receive ASTM E1394 (LIS2-A2) messages over TCP using the
ASTM E1381 (LIS01-A2) low-level protocol, and inspect them record by record.

Status: older project (2013-2018), not maintained. The window title says
"V0.0.1 Development Edition"; several features in the UI are incomplete.

## Motivation

From the original `doc/readme.txt`: when connecting a device to a LIS, the
author found no usable tools for analysing and debugging ASTM E1394
connections; the commercial tools he tried were old, vendor-specific or
expensive.

## What it does

- TCP client or server (connect / listen).
- Sends messages framed per E1381: ENQ, frames with STX, frame number,
  ETB/ETX and checksum, then EOT.
- Option to answer every received frame with NAK (error simulation).
- "Send raw": sends a text buffer line by line, with placeholders such as
  `<STX>`, `<ETB>`, `<ENQ>`, `<NAK>` replaced by the control characters.
- Loads trace files (one record per line) into a tree of sessions, messages
  and records; each field is shown and editable separately.
- Records can be added or removed manually (header, patient, order, result,
  comment, request, manufacturer, scientific, terminator).
- Projects and record profiles can be saved and loaded; traces can be saved.

Not implemented although present in the UI: serial connection. The full TODO
list is in [`doc/readme.txt`](doc/readme.txt).

## Files

```
src/astm.{h,cpp}           ASTM record model, control characters, checksum
src/culistgui.{h,cpp,ui}   main window
src/persistence.{h,cpp}    project/settings persistence
CULIST.pro                 qmake project
scripts/                   Windows release build (Visual Studio 2008 + Inno Setup)
```

## Build

Requires Qt 4 (the Windows scripts use 4.8.4; modules core, gui, network, sql); the code uses
`#include <QtGui/QApplication>` and will not build with Qt 5/6 unchanged.

```bash
qmake CULIST.pro && make
```

On Windows, `createvcprj.bat` generates a Visual Studio project and
`scripts/create_release.bat` builds a release and an installer.

## Related

- [ASTMViewer](https://github.com/vheinitz/ASTMViewer) – small C# viewer for E1394 records
- [cl2](https://github.com/vheinitz/cl2) – Qt library collection that includes a `lis/` module

## License

[MIT](LICENSE), (c) 2013 Valentin Heinitz
