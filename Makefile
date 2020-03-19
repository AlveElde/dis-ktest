obj-m += dis_ktest.o

SRC := ./src
dis_ktest-objs := $(SRC)/dis_ktest.o  		\
					$(SRC)/dis_verbs.o		\
					$(SRC)/dis_send_receive.o	\
					
EXTRA_CFLAGS += -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test:
	sudo dmesg -C
	sudo insmod dis_ktest.ko
	sudo rmmod dis_ktest.ko
	dmesg -t