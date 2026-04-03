#!/bin/sh

set -eu

# Este script usa Python em dois pontos para:
# 1) remover comentarios ';' dos .asm de teste
# 2) comparar arquivos ignorando somente newline final

echo "INFO: stripping ';' comments from ASM fixtures before assembly"

total=0
passed=0

for f in arquivos-teste/*.asm; do
	base="${f%.asm}"
	name="$(basename "$base")"
	total=$((total + 1))

	python3 - "$f" /tmp/mvn-test.asm <<'PY'
import sys
from pathlib import Path

src_path = Path(sys.argv[1])
dst_path = Path(sys.argv[2])

with src_path.open("r", encoding="utf-8") as src, dst_path.open("w", encoding="utf-8") as dst:
    for line in src:
        # Remove comentario a direita e normaliza quebra de linha
        dst.write(line.split(";", 1)[0].rstrip() + "\n")
PY

	./mnem2op /tmp/mvn-test.asm > /tmp/mvn.out

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

echo "PASS: ${passed}/${total} (test_asm)"
