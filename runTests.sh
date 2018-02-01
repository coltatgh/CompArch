#!/bin/bash
#debug
compile=$(printf ' -g -o assembler assembler.c')
gcc $compile
rm *.obj
rm *.o*
for i in *.asm; do
	a=$(printf '%s.asm %s.obj\n' "${i:0:-4}" "${i:0:-4}")
	echo $a
	./assembler $a
done


#release
 # compile=$(printf ' -ansi assembler.c')
 # gcc $compile
 # rm *.obj
 # for i in *.asm; do
 # 	a=$(printf '%s.asm %s.obj\n' "${i:0:-4}" "${i:0:-4}")
 # 	echo $a
 # 	./a.out $a
 # done
