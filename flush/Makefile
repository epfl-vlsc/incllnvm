obj-m+=gf.o

KDIR  := /lib/modules/$(shell uname -r)/build
GF   := $(shell pwd)

all: 
	$(MAKE) -C $(KDIR) M=$(GF) modules
	$(CC) testgf.c -o test
clean:
	make -C $(KDIR) M=$(GF) clean
	rm test
load:
	sudo insmod gf.ko
	sudo chmod 777 /dev/global_flush
	lsmod | grep gf
unload:
	sudo rmmod gf.ko
run:
	sudo dmesg -C
	sudo insmod gf.ko
	sudo chmod 777 /dev/global_flush
	./test
	dmesg
	sudo rmmod gf.ko
