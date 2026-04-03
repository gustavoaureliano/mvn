#!/bin/sh

set -eu

echo "INFO: running raw ASM fixtures (no preprocessing)"

total=0
passed=0

for f in arquivos-teste/*.asm; do
	base="${f%.asm}"
	name="$(basename "$base")"
	total=$((total + 1))

	if ! ./mnem2op "$f" > /tmp/mvn.out 2>/tmp/mvn.err; then
		echo "FAIL: ${name}" >&2
		cat /tmp/mvn.err >&2
		exit 1
	fi

	if ! diff -u /tmp/mvn.out "${base}.mvn" > /tmp/mvn.diff 2>&1; then
		if ! python3 - /tmp/mvn.out "${base}.mvn" <<'PY'
import sys
from pathlib import Path

left = Path(sys.argv[1]).read_bytes().rstrip(b"\n")
right = Path(sys.argv[2]).read_bytes().rstrip(b"\n")

sys.exit(0 if left == right else 1)
PY
		then
			echo "FAIL: ${name}" >&2
			cat /tmp/mvn.diff
			exit 1
		fi
	fi

	echo "PASS: ${name}"
	passed=$((passed + 1))
done

echo "PASS: ${passed}/${total} (test_asm_raw)"
