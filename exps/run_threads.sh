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
	NKEYS=20000000
	THREADS=8
	NOPS=1000000
	DELAYS=0
}

use_a_workloads() {
	WORKLOADS=(ycsb_a_uni ycsb_a_zipf)
}

remove_nvm_files() {
    rm -rf /dev/shm/* /mnt/memext4/* /scratch/tmp/*
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

write_csv_header_args() {
	#ex: $1=Nkeys
	#requires OUTFILE
	echo "TotalOps,AvgOps,StdOps,StdPOps,Workload,$1" >>${OUTFILE}
}

REPEAT=1
touch *
#ex:$1=EXTFLAGS=-DSKIP_CRITICAL_SECTION or EXTFLAGS=-DDISABLE_ALL
echo "clean then make mttest"
make clean
make mttest -j

create_output threads
write_csv_header_args Threads

use_default_params
use_a_workloads

THREAD_COUNTS=(1 2 4 6 12 18 24)
for THREADS in ${THREAD_COUNTS[@]}; do
	echo "Threads ${THREADS}"
	for WORKLOAD in ${WORKLOADS[@]}; do
		for i in $(eval echo {1..$REPEAT}); do
			remove_files
			echo "Workload:${WORKLOAD} keys and initops:${NKEYS} NOPS:${NOPS} threads:${THREADS} delay:${DELAYS} pinned"
			numactl --cpunodebind=0 ./mttest ${WORKLOAD} --nops1=${NOPS} --ninitops=${NKEYS} --nkeys=${NKEYS} --threads=${THREADS} --pin

			sleep 1

			python get_average.py "${WORKLOAD},${THREADS}" >>${OUTFILE}
			tail -n 1 ${OUTFILE}

			remove_files
		done
	done
done


cat ${OUTFILE}