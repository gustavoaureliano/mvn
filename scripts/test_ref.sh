#!/bin/sh

set -eu

if ! command -v mvn-cli >/dev/null 2>&1; then
	echo "mvn-cli not found in PATH; cannot run reference tests" >&2
	exit 1
fi

# Compare assembler output against mvn-cli for all fixtures.
asm_total=0
asm_passed=0

for f in arquivos-teste/*.asm; do
	base="$(basename "${f%.asm}")"
	asm_total=$((asm_total + 1))

	python3 - "$f" "/tmp/${base}.clean.asm" <<'PY'
import sys
from pathlib import Path

src = Path(sys.argv[1]).read_text(encoding="utf-8").splitlines()
dst = "".join(line.split(";", 1)[0].rstrip() + "\n" for line in src)
Path(sys.argv[2]).write_text(dst, encoding="utf-8")
PY

	mvn-cli assemble -i "/tmp/${base}.clean.asm" > "/tmp/${base}.ref.mvn"
	./mnem2op "/tmp/${base}.clean.asm" > "/tmp/${base}.ours.mvn"

	if ! diff -u "/tmp/${base}.ours.mvn" "/tmp/${base}.ref.mvn" > "/tmp/${base}.ref.diff" 2>&1; then
		if ! python3 - "/tmp/${base}.ours.mvn" "/tmp/${base}.ref.mvn" <<'PY'
import sys
from pathlib import Path

left = Path(sys.argv[1]).read_bytes().rstrip(b"\n")
right = Path(sys.argv[2]).read_bytes().rstrip(b"\n")
sys.exit(0 if left == right else 1)
PY
		then
			echo "Reference assembler mismatch for ${base}" >&2
			cat "/tmp/${base}.ref.diff"
			exit 1
		fi
	fi

	echo "PASS: ${base} (ref-asm)"
	asm_passed=$((asm_passed + 1))
done

echo "PASS: ${asm_passed}/${asm_total} (test_ref assembler)"

# Validate disassembler by reassembling with mvn-cli.
round_total=0
round_passed=0

for f in arquivos-teste/*.mvn; do
	base="$(basename "${f%.mvn}")"
	round_total=$((round_total + 1))

	./op2mnem "$f" > "/tmp/${base}.dis.asm"
	mvn-cli assemble -i "/tmp/${base}.dis.asm" > "/tmp/${base}.reassembled.ref.mvn"

	if ! diff -u "$f" "/tmp/${base}.reassembled.ref.mvn" > "/tmp/${base}.roundtrip.ref.diff" 2>&1; then
		if ! python3 - "$f" "/tmp/${base}.reassembled.ref.mvn" <<'PY'
import sys
from pathlib import Path

left = Path(sys.argv[1]).read_bytes().rstrip(b"\n")
right = Path(sys.argv[2]).read_bytes().rstrip(b"\n")
sys.exit(0 if left == right else 1)
PY
		then
			echo "Reference disassembler mismatch for ${base}" >&2
			cat "/tmp/${base}.roundtrip.ref.diff"
			exit 1
		fi
	fi

	echo "PASS: ${base} (ref-roundtrip)"
	round_passed=$((round_passed + 1))
done

echo "PASS: ${round_passed}/${round_total} (test_ref roundtrip)"

# Additional compatibility check: literals and labels with similar text.
compat_input="adversarial-tests/hex_literal_label_collision.asm"
if [ ! -f "$compat_input" ]; then
	echo "Missing compatibility fixture: $compat_input" >&2
	exit 1
fi

./mnem2op "$compat_input" > /tmp/compat_literal_label.ours.mvn
mvn-cli assemble -i "$compat_input" > /tmp/compat_literal_label.ref.mvn

if ! diff -u /tmp/compat_literal_label.ours.mvn /tmp/compat_literal_label.ref.mvn > /tmp/compat_literal_label.diff 2>&1; then
	echo "Reference compatibility mismatch for literal/label input" >&2
	cat /tmp/compat_literal_label.diff
	exit 1
fi

echo "PASS: literal/label compatibility (ref-asm)"
