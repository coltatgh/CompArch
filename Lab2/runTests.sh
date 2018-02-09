#!/bin/bash

rm *.obj
cd ../Lab1
compile=$(printf ' -g -o assembler assembler.c')
gcc $compile
rm *.obj
rm *.o*
for i in *.asm; do
	a=$(printf '%s.asm %s.obj\n' "${i:0:-4}" "${i:0:-4}")
	echo $a
	./assembler $a
done

find . -size 0 -delete
mv *.obj ../Lab2/

echo '------Moving to lab2 folder---------'
# cd ../Lab2/

# #debug
# compile=$(printf ' -g -o lc3bsim2 lc3bsim2.c')
# gcc $compile
# for i in *.obj; do
# 	a=$(printf '%s.obj\n' "${i:0:-4}")
# 	echo $a
# 	#./lc3bsim2 $a
# done


#release
compile=$(printf ' -ansi lc3bsim2.c')
gcc $compile
for i in *.obj; do
	a=$(printf '%s.obj\n' "${i:0:-4}")
	echo $a
	./a.out $a
done

