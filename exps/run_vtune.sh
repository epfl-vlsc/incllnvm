#!/usr/bin/env bash
rm -rf vtune/*
mkdir -p vtune

PROJ=$1
FLAGS="EXTFLAGS=-DVTUNE"

delete_files() {
    rm -rf /dev/shm/* /mnt/memext4/* ./*.json
}

use_default_params() {
    NKEYS=20000000
    THREADS=8
    NOPS=1000000
    DELAYS=0
}

if [ -z "${PROJ}" ]; then
    PROJ="vtune"
fi

touch * simple/*
make clean
make mttest $FLAGS -j
use_default_params

CMD="./mttest ycsb_e_uni --nops1=${NOPS} --ninitops=${NKEYS} --nkeys=${NKEYS} --threads=${THREADS} --pin"

source /opt/intel/vtune_amplifier/amplxe-vars.sh

delete_files

vtune -collect hotspots -knob sampling-mode=hw -knob enable-stack-collection=true -knob stack-size=0 -result-dir vtune/${PROJ}_hs -app-working-dir ./ -- ${CMD}

delete_files

skip_detailed='

vtune -collect memory-access -knob analyze-mem-objects=true -result-dir vtune/${PROJ}_ma -app-working-dir ./ -- ${CMD}

delete_files


vtune -collect-with runsa -knob enable-stack-collection=true -knob enable-user-tasks=true -knob stack-size=0 -knob event-config=CPU_CLK_UNHALTED.THREAD:sa=2000003,CYCLE_ACTIVITY.STALLS_L1D_MISS:sa=2000003,CYCLE_ACTIVITY.STALLS_L2_MISS:sa=2000003,CYCLE_ACTIVITY.STALLS_L3_MISS:sa=2000003,CYCLE_ACTIVITY.STALLS_MEM_ANY:sa=2000003,CYCLE_ACTIVITY.STALLS_TOTAL:sa=2000003,INST_RETIRED.ANY:sa=2000003 -knob enable-context-switches=true -result-dir vtune/${PROJ}_cm -app-working-dir ./ -- ${CMD}

delete_files

vtune -collect uarch-exploration -result-dir vtune/${PROJ}_ue -app-working-dir ./ -- ${CMD}

delete_files
'