#!/bin/bash

make clean 
make DEBUG=1
qemu-system-x86_64 --no-reboot -kernel kernel -serial stdio -s
