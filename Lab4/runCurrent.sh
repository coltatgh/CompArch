#!/bin/bash
cd ../Lab1
./'runTests.sh'

cd ../Lab4

echo "Do you wish to run GDB? "
select yn in "Yes" "No"; do
    case $yn in
        Yes )
			compile=$(printf ' -g -o lc3bsim4 lc3bsim4.c');
			gcc $compile;
			gdb ./lc3bsim4;
			exit;;
        No )
			compile=$(printf ' -ansi lc3bsim4.c')
			gcc $compile
			a=$(printf 'ucode4 l4Test.obj except_unknown.obj except_prot.obj except_unaligned.obj int.obj vector_table.obj')
			./a.out $a
			exit;;
    esac
done