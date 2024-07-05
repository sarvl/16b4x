#!/bin/bash

./sasm all.asm -o all.bin && ../disassembler/dasm all.bin > cmp.txt && diff all.asm cmp.txt 
