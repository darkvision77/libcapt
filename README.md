# libcapt
[![Core tests](https://github.com/darkvision77/libcapt/actions/workflows/core-tests.yml/badge.svg)](https://github.com/darkvision77/libcapt/actions/workflows/core-tests.yml)
[![Compression tests](https://github.com/darkvision77/libcapt/actions/workflows/compression-tests.yml/badge.svg)](https://github.com/darkvision77/libcapt/actions/workflows/compression-tests.yml)

Implementation of the Canon CAPT v1 protocol and SCoA compression based on reverse engineering of the original driver.

> [!IMPORTANT]
> This project is currently in an EXPERIMENTAL state.

**NOTE:** This is not a full-fledged driver that you can install into your system.
    It is an implementation of the compression algorithm and protocol,
    abstracted from any operating system or transmission channel.

## Target devices
Currently, the target devices are printers with `CNTblModel = 0` (see Canon's driver PPD).

| Model   | Cartridge     | PPM (A4) | Max Resolution | Year (approx.) |
|---------|---------------|----------|----------------|----------------|
| LBP800  | EP-22         | 8        | 600 dpi        | 1999-2001      |
| LBP810  | EP-22         | 8        | 600 dpi        | 2001-2002      |
| LBP1120 | EP-22         | 10       | 600 dpi        | 2003-2004      |
| LBP1210 | EP-25         | 14       | 600 dpi        | ~2002          |
| LBP3200 | EP-26/EP-27   | 18       | 600 dpi        | 2004-2006      |

## Compilation
```sh
cmake -S. -B build
cmake --build build
```
### Dependencies
#### Compile-time
- gcc >= 11 or clang >= 16 or MSVC (tested on 19.44.35217.0)
- cmake >= 3.21

#### Testing
- gtest (+gmock)

##### For compression tests:
- gdb
- ghostscript
- poppler-utils (`pdfseparate`, `pdfinfo`, etc.)
- captfilter deps (`libpopt0:i386` on Debian)

## Testing
### Core tests
```sh
cmake -S. -B build -DLIBCAPT_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build
```
### Compression tests
```sh
cmake -S. -B build -DLIBCAPT_BUILD_TESTS=ON -DLIBCAPT_COMPRESSION_TESTS=ON
cmake --build build
ctest --test-dir build
```
Compression test files are located at [`tests/Compression/data`](tests/Compression/data).

## Usage
There are several options if you want to print on your Canon LBP printer (except for the original driver):
- [darkvision77/captppd](https://github.com/darkvision77/captppd) — CUPS driver based on libcapt (CAPTv1 only)
- [UoWPrint](https://printserver.ink/) — print server that supports all Canon LBP models (and other printers)
- [mounaiban/captdriver](https://github.com/mounaiban/captdriver) — CUPS driver for the newer Canon LBP models (early alpha stage)

Also, the [`examples`](examples) folder contains an example program for printing PBM files. \
Keep in mind that this program does not implement all the features and is used only for printing testing.
```sh
cmake -S. -B build -DLIBCAPT_BUILD_EXAMPLES=ON
cmake --build build
./build/examples/printpbm /dev/usb/lp0 ./examples/data/example-1.pbm
```
To rasterize PDF files into PBM, you can use the [`pdf2pbm.sh`](scripts/pdf2pbm.sh) script. \
Also, you can find some test PBM files in the [`examples/data`](examples/data) folder.

## Decoder verification
The correct operation of the compressor can be verified using the decoder. \
But how can we ensure that the decoder itself is functioning correctly?

General principle:
1. The original PBM is converted to `filtered.bin` using Canon's `captfilter`, extracting its «command sequence log».
2. The decoder receives `filtered.bin`, decodes the raster, and writes the read commands to its command log.
3. If the command logs of the `captfilter` and decoder are equal, then the commands were correctly recognized.
4. The original raster is compared with the decoded one from `filtered.bin`.

Of course, the original `captfilter` does not write any logs. So...

### Fine-tuning Canon's `captfilter`
The `captfilter` is not being run directly.
Instead, it is run under a special [gdb python script](scripts/filter/wrap.py) that is responsible for:
1. **Command log:** the `captfilter` binary file contains some debug prints,
    but the DebugPrint function is disabled in the release build,
    so for each compression command there is a breakpoint that makes the corresponding entry in the log.
2. **Resolve «Buffer shift bug»:** there is a bug in `captfilter` related to the \*ThenRaw\* commands that affects the final raster. \
    Let's say there is a buffer `rawData[256]`.
    It can be represented as the following commands:
    ```c
    char* rawData;
    CopyThenRawLong(0, rawData, 255); // writes {1, 2, 3, ..., 255}
    rawData += 255;
    CopyThenRaw(0, rawData, 1); // writes {256}
    ```
    But for some reason, there is no buffer offset, and the output is something like this:
    ```c
    char* rawData;
    CopyThenRawLong(0, rawData, 255); // writes {1, 2, 3, ..., 255}
    // Oops...
    CopyThenRaw(0, rawData, 1); // writes {1} again...
    ```
    This bug can be found by compressing files with random data (see [`rand{A..F}.pbm`](tests/Compression/data)).
3. **Zero margins**: to prevent `captfilter` from resizing the raster,
    you need to set the margins to zero. But since the printer does not support zero margins,
    `captfilter` does not support them either. Therefore, we have to use the magic of gdb scripts.

## SAST Tools
[PVS-Studio](https://pvs-studio.com/pvs-studio/?utm_source=website&utm_medium=github&utm_campaign=open_source) — static analyzer for C, C++, C#, and Java code.

## License
libcapt is licensed under a 2-clause BSD license.
