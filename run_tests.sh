#!/bin/bash
set -e

make

passed=0
total=0

for infile in tests/*.in; do
    base="${infile%.in}"
    outfile="${base}.out"
    if [ ! -f "$outfile" ]; then
        continue
    fi
    total=$((total + 1))
    actual=$(./acaderp < "$infile")
    expected=$(cat "$outfile")
    if [ "$actual" = "$expected" ]; then
        echo "PASS: $(basename "$base")"
        passed=$((passed + 1))
    else
        echo "FAIL: $(basename "$base")"
        diff -u <(echo "$expected") <(echo "$actual") || true
    fi
done

echo "Passed $passed / $total tests."
