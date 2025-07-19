#!/bin/sh
set -eou pipefail

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 infile outfile"
    exit 1
fi

resolution=600
infile="$1"
outfile="$2"

gs -q -sstdout=%stderr -g4960x7040 -dNOPROMPT -dNOPAUSE -dBATCH -dSAFER -dNOMEDIAATTRS -sDEVICE=ps2write -dShowAcroForm -sOUTPUTFILE=- -sProcessColorModel=DeviceGray -sColorConversionStrategy=Gray -dLanguageLevel=3 -r$resolution -dCompressFonts=false -dNoT3CCITT -dNOINTERPOLATE -c save pop -f "$infile" |
gs -q -sstdout=%stderr -r$resolution -dNOPROMPT -dNOPAUSE -dBATCH -dSAFER -sDEVICE=pbmraw -sOutputFile=- - |
sed '/^# Image generated/d' > "$outfile"
