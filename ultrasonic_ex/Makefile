obj-m := ultrasonic.o


CROSS_COMPILE=arm-none-linux-gnueabi-

KDIR	:= /work/linux/

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules 
	$(CROSS_COMPILE)gcc -o ultrasonic main.c th_ultra.c -lpthread


install:
	cp *.ko /nfsroot/root/
	cp ultrasonic /nfsroot/root

clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean 
	rm -rf ultrasonic
	rm -rf *.o
