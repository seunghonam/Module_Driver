obj-m := dht11.o


CROSS_COMPILE=arm-none-linux-gnueabi-

KDIR	:= /work/linux/

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules 
	$(CROSS_COMPILE)gcc -o dht11 main.c th_dht11.c -lpthread


install:
	cp *.ko /nfsroot/root/
	cp dht11 /nfsroot/root

clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean 
	rm -rf dht11
	rm -rf *.o
