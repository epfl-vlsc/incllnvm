#!/usr/bin/env bash

delete_nvm() {
    rm -rf /dev/shm/* /mnt/memext4/*
}

delete_nvm

OUT=output.txt
mode=$1
exe=test_${mode}.out
rm ${OUT} ${exe}

reset
touch *
make ${exe} -j

MEM_MODES=(dram nvm)
RAND_MODES=(uni seq)
REPEAT=2

echo "time_ms,ops_sec,memory,rand" >${OUT}

for MM in ${MEM_MODES[@]}; do
    for RM in ${RAND_MODES[@]}; do
            for i in $(eval echo {1..$REPEAT}); do
                CMD="./${exe} ${MM} ${RM}"

                delete_nvm
                echo ${CMD}
                numactl --cpunodebind=0 ${CMD} >>${OUT}

                delete_nvm

                tail -n 1 ${OUT}
            done
        done
done

cat ${OUT}
