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

remove_nvm_files() {
    rm -rf /scratch/tmp/nvm.*
    rm -rf /dev/shm/incll/nvm.*
    rm -rf /mnt/memext4/incll/*
}

remove_json_out() {
    rm -rf *.json
}

NB_SRC=notebook-mttest.json
NB_DST=output/recovery.json

REPEAT=1
touch *
make clean
make mttest -j

rm -rf *.json
for WORKLOAD in ycsb_a_uni_recovery ycsb_a_zipf_recovery; do
    echo ${WORKLOAD}
	for i in $(eval echo {1..$REPEAT}); do 
		echo "recovery ${WORKLOAD} ${i}"
		remove_nvm_files

		./mttest ${WORKLOAD} --nops1=1000000 --ninitops=1000000 --nkeys=1000000 -j8
		echo -e "\n\n\n\n"
		./mttest ${WORKLOAD} --nops1=1000000 --ninitops=1000000 --nkeys=1000000 -j8
		sleep 1
	done
done

remove_nvm_files
cp ${NB_SRC} ${NB_DST}
