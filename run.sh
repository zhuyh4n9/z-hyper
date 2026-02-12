#!/bin/bash

CPU="cortex-a72"
QCPU="cortex-a72"
QEMU=qemu-system-aarch64
GIC_VERSION=3
MACHINE="virt,gic-version=$GIC_VERSION,virtualization=on"
NCPU=1
QEMUOPTS="-cpu $QCPU -machine $MACHINE -smp $NCPU -m 256M -nographic\
          -kernel build/z-hyper.elf"

if [ "$1" == "debug" ]; then
    echo "waiting for gdb..."
    $QEMU $QEMUOPTS -S -gdb tcp::7788
elif [ "$1" == "kill" ]; then
    echo "kill all qemu instances ..."
    kill -9 `pidof qemu-system-aarch64`
else
    echo "run mode ..."
    $QEMU $QEMUOPTS
fi
