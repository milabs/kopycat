#!/bin/sh

set -e

[ ! -f ${1} ] && exit -1

echo "unsigned char payload[] = {"
objcopy --dump-section .text=/dev/stdout ${1} | xxd -i
echo "};"

echo "unsigned payload_len = sizeof(payload);"
