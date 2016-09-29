#!/bin/bash

make clean
make
qemu-system-x86_64 --no-reboot -kernel kernel -serial stdio
