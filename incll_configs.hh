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

/*
 * Parameters of the system
 * Enable/disable #defines
 */

#pragma once

// Debug Logging
//#define DBG
#ifdef DBG
#define REC_ASSERT(cond) assert(cond);
#define DBGLOG(f_, ...)                                                        \
  printf(("At %s:%d " f_ "\n"), __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define REC_ASSERT(cond)
#define DBGLOG(f_, ...)
#endif

//------------------------------------------------------------------------------------
// default epoch 16
#define GL_FREQ 16
#define YCSB
//#define YCSB_CPNVM

// all defines
//#define BASELINEMT
#define USE_OPTANE

#define PALLOCATOR
#ifdef BASELINEMT
#define KEY_LW 15
#define KEY_MID KEY_LW / 2
#define PALLOCATOR_NOHEADER
#else // BASELINEMT
#define KEY_LW 14
#define KEY_MID KEY_LW / 2
#define GLOBAL_FLUSH
#define INCLL
#define EXTLOG
#endif // BASELINEMT

#ifdef USE_OPTANE
#ifndef MAP_SYNC
#define MAP_SYNC 0x80000
#endif
#ifndef MAP_SHARED_VALIDATE
#define MAP_SHARED_VALIDATE 0x03
#endif
#endif // optane

#ifdef EXTLOG
#define LN_EXTLOG
#define LN_EXTLOG_INCLL
#define IN_EXTLOG
#endif // extlog

//#define PERF_WORKLOAD
#define REMOVE_HEAP

// disable dealloc for remove
//#define DISABLE_DEALLOC

// stats and recovery
//#define MTAN
//#define YCSB_RECOVERY
//#define EXTLOG_STATS
//#define GF_STATS

#ifdef PALLOCATOR

#ifdef USE_OPTANE
#define PLOG_FILENAME "/mnt/memext4/incll/nvm.log"
#define PDATA_FILENAME "/mnt/memext4/incll/nvm.data"
#define PDATA_DIRNAME "/mnt/memext4/incll/"
#else
#define PLOG_FILENAME "/dev/shm/incll/nvm.log"
#define PDATA_FILENAME "/dev/shm/incll/nvm.data"
#define PDATA_DIRNAME "/dev/shm/incll/"
#endif

#define DATA_BUF_SIZE (1ull << 32)
#define DATA_REGION_ADDR (1ull << 45)
#define DATA_MAX_THREAD 10

#define PBUF_SIZE (1ull << 28)
#define LOG_REGION_ADDR (1ull << 34)
#define LOG_MAX_THREAD 10

#endif // PALLOCATOR

#ifndef VTUNE
// zero overhead
#define INTEL_NO_ITTNOTIFY_API
#else
#include <ittnotify.h>
#endif