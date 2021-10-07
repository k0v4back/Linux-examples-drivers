obj-m := main.o

HOST_KERN_DIR = /lib/modules/5.4.0-84-generic/build/

all:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules

clean:
	make -C $(HOST_KERN_DIR) M=$(PWD) clean
