#!/bin/bash
#debug
compile=$(printf ' -g -o assemble assemble.c')
gcc $compile
rm *.obj
for i in *.asm; do
	a=$(printf '%s.asm %s.obj\n' "${i:0:-4}" "${i:0:-4}")
	echo $a
	./assemble $a
done


#release
# compile=$(printf ' -ansi assemble.c')
# gcc $compile
# rm *.obj
# for i in *.asm; do
# 	a=$(printf '%s.asm %s.obj\n' "${i:0:-4}" "${i:0:-4}")
# 	echo $a
# 	./a.out $a
# done