#!/bin/sh

cat | ./k++ --ir | llc -o test.s && gcc test.s -o test && ./test $@; echo $?
