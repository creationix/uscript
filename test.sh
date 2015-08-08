#!/bin/sh
set -e
clang -Wall -Werror -std=c99 -Os e13.c
time ./a.out
