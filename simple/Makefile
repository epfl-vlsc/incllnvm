SRC_DIR := ./
OBJ_DIR := ./

CFLAGS := -D_GNU_SOURCE -g -O2 -std=c++17
IFLAGS := -I../logs -I../allocs -I../chpts -I../configs -I../utils -I../inits -I.. -I../locks
LFLAGS := -lpthread
			
all: $(patsubst %.cpp, %.out, $(wildcard *.cpp))

%.out: $(SRC_DIR)/%.cpp
	g++ $(CFLAGS) $(IFLAGS) $< -o $@ $(LFLAGS)

clean:
	rm -f *.o *.a *.out
	rm -rf /dev/shm/nvmcp/* /mnt/memext4/nvmcp/*