#!/bin/sh

set -eu

if ! command -v mvnMonitor >/dev/null 2>&1; then
	echo "mvnMonitor not found in PATH; cannot run monitor tests" >&2
	exit 1
fi

if ! command -v mvn-cli >/dev/null 2>&1; then
	echo "mvn-cli not found in PATH; cannot build monitor reference binaries" >&2
	exit 1
fi

for f in arquivos-teste/*.asm; do
	base="$(basename "${f%.asm}")"

	python3 - "$f" "/tmp/${base}.clean.asm" <<'PY'
import sys
from pathlib import Path

src = Path(sys.argv[1]).read_text(encoding="utf-8").splitlines()
dst = "".join(line.split(";", 1)[0].rstrip() + "\n" for line in src)
Path(sys.argv[2]).write_text(dst, encoding="utf-8")
PY

	mvn-cli assemble -i "/tmp/${base}.clean.asm" > "/tmp/${base}.ref.mvn"
	./mnem2op "/tmp/${base}.clean.asm" > "/tmp/${base}.ours.mvn"

	cp "/tmp/${base}.ref.mvn" /tmp/prog.mvn
	mvnMonitor <<'EOF' > "/tmp/${base}.monitor.ref.txt"
p /tmp/prog.mvn
r

n
g
m 0300 0310
x
EOF

	cp "/tmp/${base}.ours.mvn" /tmp/prog.mvn
	mvnMonitor <<'EOF' > "/tmp/${base}.monitor.ours.txt"
p /tmp/prog.mvn
r

n
g
m 0300 0310
x
EOF

	if ! diff -u "/tmp/${base}.monitor.ref.txt" "/tmp/${base}.monitor.ours.txt" > "/tmp/${base}.monitor.diff" 2>&1; then
		echo "Monitor execution mismatch for ${base}" >&2
		cat "/tmp/${base}.monitor.diff"
		exit 1
	fi
done
