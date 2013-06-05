#!/bin/sh

cat | ./k++ --ir | llc -o test.s && gcc test.s ../src/krt.c -o test && ./test $@; echo $?
