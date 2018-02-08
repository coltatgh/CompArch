#!/bin/bash
#debug
compile=$(printf ' -g -o lc3bsim2 lc3bsim2.c')
gcc $compile
for i in *.obj; do
	a=$(printf '%s.obj\n' "${i:0:-4}")
	echo $a
	./lc3bsim2 $a
done


#release
# compile=$(printf ' -ansi lc3bsim2.c')
# gcc $compile
# for i in *.obj; do
# 	a=$(printf '%s.obj\n' "${i:0:-4}")
# 	echo $a
# 	./a.out $a
# done

