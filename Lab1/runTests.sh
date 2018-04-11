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
	b=$(printf '%s.obj' "${i:0:-4}")
	c=$(printf '../Lab4/%s' "${b}")
	mv $b $c
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
