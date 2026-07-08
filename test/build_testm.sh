#!/bin/bash

C_TEST_SF=can_do_succ_endecb.c      # C source file (test)
C_TEST_OF=can_do_succ_endecb        # C output binary file (test) 
C_TEST_OBF=hardware_engln_board.bin # C object file (test) 


# Optional
clear

# Clear leftover c object binary files (from main)
rm $C_TEST_OBF

# Compile with sanitizer, especially memory leaks
clang -I../include $C_TEST_SF ../include/backend/file_io.c ../include/backend/endec_board.c -o $C_TEST_OF -g -fsanitize=address

# Run
./$C_TEST_OF

# HEXDUMP for analysis
hexdump $C_TEST_OBF
