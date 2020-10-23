# Durable Masstree #

This is the experimental source release for Durable Masstree, a fast, multi-core key-value store that runs on NVM.
Ensuring the recoverability of the data after crash is expensive due to the cost involved in ordering the writes to NVM.
Durable Masstree offers a new design space, where the cost of ordering writes by using flushes and fences can be reduced through the use of fine-grain checkpointing and using a log that is stored within a cache line as described in Fine-Grain Checkpointing with In-Cache-Line Logging.
Durable Masstree code offers the possibility to explore the described design space.
  
This document describes how to run Durable Masstree and obtain results.

## Dependencies ##

* jemalloc

## Installation ##

Get the sources using:
	
	$ git clone https://github.com/epfl-vlsc/Masstree.git

Please make sure you have jemalloc installed.
Afterwards, run the commands below to build the project.

    $ bash exps/run_init.sh
    $ bash exps/run_workload.sh
    $ bash exps/run_workloads.sh

## Cache line flusher ##

Load the kernel module for kernel flushes.

    $ make flush_all
    $ make flush_load
    
## Persistent Region Size ##
* In `incll_configs.hh` please change `DATA_BUF_SIZE, PDATA_FILENAME, PDATA_DIRNAME` and please change, `PBUF_SIZE, PLOG_FILENAME` according to the mapping location and size of the memory available to your system.

## References ##

	Cohen, Aksun, Avni, Larus, "Fine-Grain Checkpointing with In-Cache-Line Logging," ASPLOS 2019.
