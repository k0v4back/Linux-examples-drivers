CONFIG_MODULE_SIG=n

obj-m := platform_driver.o device_setup.o

HOST_KERN_DIR = /lib/modules/$(shell uname -r)/build/

all:
		make -C $(HOST_KERN_DIR) M=$(PWD) modules

clean:
		make -C $(HOST_KERN_DIR) M=$(PWD) clean
