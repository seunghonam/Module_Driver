obj-m := linetracer.o


CROSS_COMPILE=arm-none-linux-gnueabi-

KDIR	:= /work/linux/

all:
	make -C $(KDIR) SUBDIRS=$(PWD) modules 
	$(CROSS_COMPILE)gcc -o linetracer main.c th_linetracer.c -lpthread


install:
	cp *.ko /nfsroot/root/
	cp linetracer /nfsroot/root

clean:
	make -C $(KDIR) SUBDIRS=$(PWD) clean 
	rm -rf linetracer
	rm -rf *.o
