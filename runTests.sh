#!/bin/bash
compile=$(printf ' -g -o assemble assemble.c')
gcc $compile
for i in *.asm; do
	a=$(printf '%s.asm %s.obj\n' "${i:0:-4}" "${i:0:-4}")
	./assemble $a
done