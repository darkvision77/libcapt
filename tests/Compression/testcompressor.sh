#!/bin/bash
set -eou pipefail
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 compressorexe decoderexe orig.pbm"
    exit 1
fi

compressor="$1"
decoder="$2"
origpbm="$3"

cmp "$origpbm" <($decoder <($compressor "$origpbm" /dev/stdout) /dev/stdout /dev/null)
