obj-m += dis_ktest.o

SRC := ./src
dis_ktest-objs := $(SRC)/dis_ktest.o  			\
					$(SRC)/dis_send_receive.o	\
					# $(SRC)/dis_verbs.o		\
					
EXTRA_CFLAGS += -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

dmesg-c: 
	sudo dmesg -C

test: dmesg-c
	sudo insmod dis_ktest.ko
	sudo rmmod dis_ktest.ko
	dmesg -t

ins-rxe:
	sudo modprobe rdma_rxe
	sudo bash -c "echo enp1s0 > /sys/module/rdma_rxe/parameters/add"

rm-rxe:
	sudo bash -c "echo rxe0 > /sys/module/rdma_rxe/parameters/remove"
	sudo modprobe -r rdma_rxe

rxe: dmesg-c ins-rxe test rm-rxe
	dmesg -t

record:
	sudo trace-cmd record -p function_graph -l 'dis_qp_notify'

report:
	sudo trace-cmd report