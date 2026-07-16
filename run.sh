#!/bin/bash

CPU="cortex-a72"
QCPU="cortex-a72"
QEMU=qemu-system-aarch64
GIC_VERSION=3
MACHINE="virt,gic-version=$GIC_VERSION,virtualization=on"
NCPU=1
RUN_MODE="normal"

# run.sh [-r|-d|-k] [-c NCPU] [elf_file]
while getopts "rdkc:h" opt; do
    case $opt in
        r) RUN_MODE="normal" ;;
        d) RUN_MODE="debug" ;;
        k) RUN_MODE="kill" ;;
        c) NCPU="$OPTARG" ;;
        h)
            echo "usage: $0 [-r|-d|-k] [-c NCPU] [elf_file]"
            echo "  -r  run normally"
            echo "  -d  run in debug mode (gdb waiting for connection on localhost:7788)"
            echo "  -k  kill the QEMU process"
            echo "  -c  number of CPUs (default: 1)"
            echo "  elf_file  ELF file to load (default: build/z-hyper.elf)"
            exit 0
            ;;
        *)
            echo "usage: $0 [-r|-d|-k] [-c NCPU] [elf_file]"
            exit 1
            ;;
    esac
done
shift $((OPTIND-1))

ELF_FILE="${1:-build/z-hyper.elf}"

echo "RUN_MODE=$RUN_MODE, ELF_FILE=$ELF_FILE, NCPU=$NCPU"

QEMUOPTS="-cpu $QCPU -machine $MACHINE -smp $NCPU -m 256M -nographic \
          -kernel $ELF_FILE"

if [ "$RUN_MODE" = "kill" ]; then
    echo "killing qemu..."
    pkill -f "$QEMU"
    exit 0
elif [ "$RUN_MODE" = "debug" ]; then
    echo "gdb waiting for connection on localhost:7788"
    $QEMU $QEMUOPTS -S -gdb tcp::7788
    exit 0
else
    $QEMU $QEMUOPTS
fi
