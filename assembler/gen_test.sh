#!/bin/bash

cat -n tests/code/$1.sasm 
./sasm tests/code/$1.sasm -o tests/fout/$1.txt > tests/sout/$1.txt 
touch tests/fout/$1.txt
echo "sout:"
cat tests/sout/$1.txt
echo "fout dis:"
../dasm tests/fout/$1.txt
