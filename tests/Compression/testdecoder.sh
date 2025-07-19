#!/bin/bash
set -eou pipefail
if [ "$#" -ne 4 ]; then
    echo "Usage: $0 decoderexe orig.bin orig.pbm orig.log"
    exit 1
fi

decoder="$1"
origbin="$2"
origpbm="$3"
origlog="$4"
decpbm="$origbin-decoded.pbm"
declog="$origbin-decoded.log"

$decoder "$origbin" "$decpbm" "$declog"
cmp "$origlog" "$declog" && echo "cmd log OK" || exit 1
cmp "$origpbm" "$decpbm" && echo "raster OK" || exit 1
