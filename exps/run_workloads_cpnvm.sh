#!/usr/bin/env bash

: '
/* Modification of Masstree
 * VLSC Laboratory
 * Copyright (c) 2018-2019 Ecole Polytechnique Federale de Lausanne
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Masstree LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Masstree LICENSE file; the license in that file
 * is legally binding.
 */
'

use_default_params() {
    NKEYS=90000000
    THREADS=6
    NOPS=1000000
    DELAYS=0
}

use_all_workloads() {
    WORKLOADS=(ycsb_a_uni ycsb_b_uni ycsb_c_uni ycsb_a_zipf ycsb_b_zipf ycsb_c_zipf)
}

remove_nvm_files() {
    rm -rf /scratch/tmp/nvm.*
    rm -rf /dev/shm/incll/nvm.*
    rm -rf /mnt/memext4/incll/*
}

remove_json_out() {
    rm -rf *.json
}

remove_files() {
    remove_nvm_files
    remove_json_out
}

create_output() {
    #$1=filename
    mkdir -p output
    OUTFILE=output/$1.txt
    rm -rf ${OUTFILE}
    echo "Results in ${OUTFILE}"
}

write_csv_header(){
	#requires OUTFILE
	echo "TotalOps,AvgOps,StdOps,StdPOps,Workload" >> ${OUTFILE}
}

REPEAT=1
touch *
#ex:$1=EXTFLAGS=-DSKIP_CRITICAL_SECTION or EXTFLAGS=-DDISABLE_ALL
echo "clean then make mttest"
make clean
make mttest -j

create_output workloads
write_csv_header

use_default_params
use_all_workloads

for WORKLOAD in ${WORKLOADS[@]}; do
    for i in $(eval echo {1..$REPEAT}); do
        remove_files
        echo "Workload:${WORKLOAD} keys and initops:${NKEYS} NOPS:${NOPS} threads:${THREADS} delay:${DELAYS} pinned"
        numactl --cpunodebind=0 ./mttest ${WORKLOAD} --nops1=${NOPS} --ninitops=${NKEYS} --nkeys=${NKEYS} --threads=${THREADS} --pin

        sleep 1

        python get_average.py "${WORKLOAD}" >>${OUTFILE}
        tail -n 1 ${OUTFILE}

        remove_files
    done
done

cat ${OUTFILE}
