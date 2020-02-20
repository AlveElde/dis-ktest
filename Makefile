obj-m += dis_ktest.o

SRC := ./src
dis_ktest-objs := $(SRC)/dis_ktest.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

ins: 
	sudo dmesg -C
	sudo insmod dis_ktest.ko
	dmesg

rm: 
	sudo dmesg -C
	sudo rmmod dis_ktest.ko
	dmesg

test: ins rm