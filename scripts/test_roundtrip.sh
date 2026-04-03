#!/bin/sh

set -eu

# Este script usa Python apenas para comparar ignorando newline final.

total=0
passed=0

for f in arquivos-teste/*.mvn; do
	base="$(basename "${f%.mvn}")"
	total=$((total + 1))

	./op2mnem "$f" > "/tmp/${base}.dis.asm"
	./mnem2op "/tmp/${base}.dis.asm" > "/tmp/${base}.roundtrip.mvn"

	if ! diff -u "/tmp/${base}.roundtrip.mvn" "$f" > /tmp/mvn.diff 2>&1; then
		if ! python3 - "/tmp/${base}.roundtrip.mvn" "$f" <<'PY'
import sys
from pathlib import Path

left = Path(sys.argv[1]).read_bytes().rstrip(b"\n")
right = Path(sys.argv[2]).read_bytes().rstrip(b"\n")

sys.exit(0 if left == right else 1)
PY
		then
			echo "FAIL: ${base}" >&2
			cat /tmp/mvn.diff
			exit 1
		fi
	fi

	echo "PASS: ${base}"
	passed=$((passed + 1))
done

echo "PASS: ${passed}/${total} (test_roundtrip)"
