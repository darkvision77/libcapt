#!/bin/bash
set -eou pipefail

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 pages outfile"
    exit 1
fi

for i in $(seq $1); do
    cat <(printf 'P4\n4960 7040\n') <(dd if=/dev/zero of=/dev/stdout bs=620 count=7040)
done > "$2"
