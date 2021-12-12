obj-m := platform_driver_syscalls.o

platform_driver_syscalls-objs += platform_driver_dt_sysfs.o platform_driver_syscalls.o

ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
KERN_DIR=/home/k0v4/coding/linux/kernals/linux_bbb_5.4/
HOST_KERN_DIR = /lib/modules/$(shell uname -r)/build/

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) clean

help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) help

host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules

copy-dtb:
	scp $(KERN_DIR)arch/arm/boot/dts/am335x-boneblack.dtb debian@192.168.1.184:/home/debian/drivers
