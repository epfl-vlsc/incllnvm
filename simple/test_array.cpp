#include "xdistribution.h"
#include "xmemmap.h"
#include <vector>

#include <chrono>
#include <iostream>

#define Addr (1ull << 40)
size_t n;
using ms = std::chrono::milliseconds;

memmap_t *region;
const size_t sz = (1ull << 27);
size_t *arr;
size_t key;
size_t val;
std::vector<size_t> keys;
std::vector<size_t> vals;
int64_t ts;
double thpt;

static inline void clflushopt(volatile void *__p) {
  __asm__ volatile(".byte 0x66; clflush %0" : "+m"(*(volatile char *)(__p)));
}

static inline void sfence() { __asm__ volatile("sfence" ::: "memory"); }

void fill(bool is_uni, size_t sz) {
  n = sz / sizeof(long);
  UniformDist uni_key(n, 0);
  UniformDist uni_val(n, 1000);

  keys.reserve(n);
  vals.reserve(n);

  for (int i = 0; i < n; ++i) {
    if (is_uni) {
      keys[i] = uni_key.next() % n;
    } else {
      keys[i] = (i * 8) % n;
    }

    vals[i] = uni_val.next();
  }
}

void write() {
  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < n; ++i) {
    key = keys[i];
    val = vals[i];
    arr[key] = val;
    clflushopt(arr + key);
  }
  sfence();
  auto end = std::chrono::steady_clock::now();
  ts = std::chrono::duration_cast<ms>(end - start).count();
  thpt = (double)n * 1000 / (double)ts;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("wrong arguments");
    exit(1);
  }

  const char *mem_mode = argv[1];
  const char *rand_mode = argv[2];

  bool nv = (strcmp(mem_mode, "nvm") == 0);
  bool un = (strcmp(rand_mode, "uni") == 0);

  void *addr = (void *)(uintptr_t)Addr;
  region = new memmap_t("test", sz, addr, nv);
  arr = (size_t *)region->get_base();
  fill(un, sz);
  write();

  printf("%lu,%0.3f,%s,%s\n", ts, thpt, mem_mode, rand_mode);
}