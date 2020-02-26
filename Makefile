obj-m += dis_ktest.o

SRC := ./src
dis_ktest-objs := $(SRC)/dis_ktest.o  		\
					$(SRC)/dis_verbs.o		\
					$(SRC)/dis_requester.o  \
					$(SRC)/dis_responder.o	\
					

EXTRA_CFLAGS += -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

install:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules_install

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

req: 
	sudo dmesg -C
	sudo insmod dis_ktest.ko is_responder=N
	sudo rmmod dis_ktest.ko
	dmesg

res: 
	sudo dmesg -C
	sudo insmod dis_ktest.ko is_responder=Y
	sudo rmmod dis_ktest.ko
	dmesg

full: clean all req res