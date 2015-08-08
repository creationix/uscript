#!/bin/sh
set -e
cc -Wall -Werror -std=c99 -Os e13.c
time ./a.out
