#!/bin/bash -
#===============================================================================
#
#          FILE: gen_read.sh
#
#         USAGE: ./gen_read.sh
#
#   DESCRIPTION:
#
#       OPTIONS: ---
#  REQUIREMENTS: ---
#          BUGS: ---
#         NOTES: ---
#        AUTHOR: YOUR NAME (),
#  ORGANIZATION:
#       CREATED: 2021年03月29日 14:45
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

arm-none-eabi-gcc -mcpu=cortex-m33 -E read_intrinsics_tmpl.c -I../include/beco/ > tmp.c

awk 'BEGIN{
        printf "/*\n";
        printf " * NOTE:\n";
        printf " * Dont change this file, it is generated by gen_read.sh\n";
        printf " */\n\n";
        }

    /beco_read_acc*.*\}$/ {print $0}
    END {printf "\n"}' tmp.c >  read_intrinsics.c

./Bindent read_intrinsics.c

rm tmp.c
