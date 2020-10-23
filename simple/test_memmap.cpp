#include "xmemmap.h"

#include <chrono>

using ms = std::chrono::milliseconds;
const size_t n = (1ul << 28);
const int s = sizeof(int) * n;
int *arr;

memmap_t *region_int;
int64_t ts;

static inline void clflushopt(volatile void *__p) {
  __asm__ volatile(".byte 0x66; clflush %0" : "+m"(*(volatile char *)(__p)));
}

static inline void sfence() { __asm__ volatile("sfence" ::: "memory"); }

void init(bool nv) {
  void *p = (void *)(1ull << 39);
  region_int = new memmap_t("int1", s, p, nv);
  arr = (int *)region_int->memmap_alloc(s);
}

void run() {
  for (int i = 0; i < n; ++i) {
    arr[i] = i;
    if (i % 16 == 0) {
      clflushopt(arr + i);
    }
  }
  sfence();
}

void end() { region_int->destroy_file(); }

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("wrong arguments");
    exit(1);
  }

  bool nv = (strcmp(argv[1], "nvm") == 0);
  init(nv);

  auto start = std::chrono::steady_clock::now();
  run();
  auto end = std::chrono::steady_clock::now();
  ts = std::chrono::duration_cast<ms>(end - start).count();

  printf("%lu,%ld\n", ts, nv);

  return 0;
}