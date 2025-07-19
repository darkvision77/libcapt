#!/bin/sh
set -e
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 infile cmdfile outfile"
    exit 1
fi

infile=$(readlink -f "$1")
cmdfile=$(readlink -f "$2")
outfile=$(readlink -f "$3")
filter="$(dirname -- "$0")/captfilter"

echo "$infile"

export WRAP_CMDFILE="$cmdfile"
export WRAP_OUTFILE="$outfile"
export WRAP_INFILE="$infile"
export WRAP_FILTEREXE="$filter"
gdb --batch --command="$(dirname -- "$0")/wrap.py"
