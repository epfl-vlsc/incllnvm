#pragma once

#include <atomic>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "spinlock.h"

#ifndef MAP_SYNC
#define MAP_SYNC 0x80000
#endif

#ifndef MAP_SHARED_VALIDATE
#define MAP_SHARED_VALIDATE 0x03
#endif

class memmap_t {
  static constexpr const int closed_file = -1;
  static constexpr const int removed_file = 0;
  static constexpr const int fname_size = 100;
  static constexpr const char *nvm_dir = "/mnt/memext4/nvmcp";
  static constexpr const char *dram_dir = "/dev/shm/nvmcp";

protected:
  // mmap functionality
  char fname_[fname_size];
  int fd_;
  void *base_;
  size_t size_;
  bool exists_;
  bool on_nvm_;

  // malloc functionality
  nspinlock_t malloc_lock_;
  char *malloc_addr_;
  size_t used_space_;

  bool path_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
  }

  void init_paths(const char *filename) {
    const char *memmap_dir = (on_nvm_) ? nvm_dir : dram_dir;

    mkdir(memmap_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if (!path_exists(memmap_dir)) {
      fprintf(stderr, "Error creating %s\n", memmap_dir);
      perror("memmap");
      exit(1);
    }

    snprintf(fname_, fname_size, "%s/%s.nvm", memmap_dir, filename);
  }

  void test_write() {
    if (!exists_) {
      int val = 0;
      lseek(fd_, size_, SEEK_SET);
      assert(write(fd_, (void *)&val, sizeof(val)) == sizeof(val));
      lseek(fd_, 0, SEEK_SET);
    }
  }

  void open_file() {
    if (on_nvm_) {
      fd_ = open(fname_, O_RDWR | O_CREAT | O_DIRECT,
                 S_IRWXU | S_IRWXG | S_IRWXO);
    } else {
      fd_ = open(fname_, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    }

    if (fd_ == closed_file) {
      perror("open file");
      exit(1);
    }

    if (!exists_)
      test_write();
  }

  void memmap_file(void *addr) {
    if (on_nvm_) {
      base_ = mmap(addr, size_, PROT_READ | PROT_WRITE,
                   MAP_SHARED_VALIDATE | MAP_SYNC, fd_, 0);
    } else {
      base_ = mmap(addr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    }

    fallocate(fd_, 0, 0, size_);
    if (base_ == MAP_FAILED) {
      fprintf(stderr, "Memmap failed %d\n", fd_);
      perror("memmap");
      exit(-1);
    }
    if (base_ != addr) {
      fprintf(stderr, "Memmapped to wrong address: expected %p got %p\n", addr,
              base_);
      perror("memmap");
      exit(-1);
    }

    malloc_addr_ = (char *)base_;

    map_populate_space();
  }

  void remove_file() {
    if (path_exists(fname_)) {
      if (remove(fname_) != removed_file) {
        fprintf(stderr, "Failed to remove %s\n", fname_);
      }
    }
  }

  void map_populate_space() {
    char *cur = (char *)base_;
    char *tmp = nullptr;
    volatile int val = 0;
    for (size_t i = 0; i < size_; i += 4096) {
      tmp = cur + i;
      val = *((int *)tmp);
      *((int *)tmp) = val;
    }
  }

public:
  memmap_t(const char *filename, size_t size, void *addr, bool on_nvm = false)
      : size_(size), on_nvm_(on_nvm) {

    init_paths(filename);
    exists_ = path_exists(fname_);
    open_file();
    memmap_file(addr);
#ifdef DBG_MEMMAP
    print();
#endif
  }

  ~memmap_t() {
    if (base_ != nullptr && size_ > 0)
      munmap(base_, size_);

    if (fd_ != closed_file)
      close(fd_);
  }

  void destroy_file() {
    assert(base_ != nullptr && size_ > 0);
    munmap(base_, size_);

    assert(fd_ != closed_file);
    close(fd_);

    remove_file();
  }

  void clear_memmap() {
    assert(base_);
    memset(base_, 0, size_);
  }

  int get_fd() const { return fd_; }

  void print() const {
    const char *exists_str = (exists_) ? "Found" : "Created";
    double total_space = (size_ * 1.0) / (1ul << 20);
    char *end = (char *)base_ + size_;
    printf("%s %s at %p to %p total_space:%0.fM used_space:%lu fd:%d\n",
           exists_str, fname_, base_, end, total_space, used_space_, fd_);
  }

  void print_malloc() const {
    printf("%s malloc_addr:%p used_space:%lu\n", fname_, malloc_addr_,
           used_space_);
  }

  void *memmap_alloc(size_t sz) {
    void *ptr;
    malloc_lock_.lock();

    ptr = (void *)malloc_addr_;
    malloc_addr_ += sz;
    used_space_ += sz;
    if (ptr == nullptr || used_space_ > size_) {
      fprintf(stderr,
              "%s Malloc overflow at ptr:%p used:%lu, total space:%lu\n",
              fname_, ptr, used_space_, size_);
      exit(-1);
    }

    malloc_lock_.unlock();
    return ptr;
  }

  bool file_exists() const { return exists_; }

  size_t get_total_space() const { return size_; }

  size_t get_used_space() const { return used_space_; }

  void set_used_space(size_t used_space) {
    assert(used_space <= size_);
    used_space_ = used_space;
    malloc_addr_ = ((char *)base_) + used_space;
  }

  void *get_base() { return base_; }

  char *get_begin() { return (char *)base_; }

  char *get_end() { return ((char *)base_) + size_; }

  void copy_to(memmap_t *memmap) {
    void *to_base = memmap->get_base();
    memcpy(to_base, base_, size_);
    memmap->set_used_space(used_space_);
  }

  bool in_region(char *addr) const {
    char *begin = (char *)base_;
    char *end = ((char *)base_) + size_;
    return begin <= addr && addr < end;
  }
};
