#!/bin/bash

export LC_ALL=C.UTF-8
export LANG=C.UTF-8

CUR_DIR=$(pwd)
IMAGE=$(pwd)/Image

usage() {
    echo "usage: ./build.sh <cpu_id 0~3 or all>"
}

# TODO: Please plan the memory according to the actual needs of the project.
# Requiring 2M alignment.
CPU0_MEM_BASE=0x04000000
CPU1_MEM_BASE=0x00800000
CPU2_MEM_BASE=0x01000000
CPU3_MEM_BASE=0x01800000
CPU4_MEM_BASE=0x02000000
CPU5_MEM_BASE=0x02800000
CPU6_MEM_BASE=0x03000000
CPU7_MEM_BASE=0x03800000
CPU0_MEM_SIZE=0x00800000
CPU1_MEM_SIZE=0x00800000
CPU2_MEM_SIZE=0x00800000
CPU3_MEM_SIZE=0x00800000
CPU4_MEM_SIZE=0x00800000
CPU5_MEM_SIZE=0x00800000
CPU6_MEM_SIZE=0x00800000
CPU7_MEM_SIZE=0x00800000

make_rtt() {
    export RTT_PRMEM_BASE=$(eval echo \$CPU$1_MEM_BASE)
    export RTT_PRMEM_SIZE=$(eval echo \$CPU$1_MEM_SIZE)
    export RTT_SHMEM_BASE=0x07800000
    export RTT_SHMEM_SIZE=0x00400000
    export LINUX_RPMSG_BASE=0x07c00000
    export LINUX_RPMSG_SIZE=0x00600000
    export CUR_CPU=$1
    scons -c
    rm -rf $CUR_DIR/gcc_arm.ld $IMAGE/amp.img $IMAGE/rtt$1.elf $IMAGE/rtt$1.bin
    scons -j32
    cp $CUR_DIR/rtthread.elf $IMAGE/rtt$1.elf
    mv $CUR_DIR/rtthread.bin $IMAGE/rtt$1.bin
}

case $1 in
    0|1|2|3|4|5|6|7)
        make_rtt $1
        ;;
    all)
        for i in {0..7};
            do make_rtt $i;
        done
        ;;
    *)
        usage; exit ;;
esac

#./mkimage.sh
