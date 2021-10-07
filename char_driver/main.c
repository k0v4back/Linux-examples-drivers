#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>

//Number of devices(Minor devices)
#define NUM_DEV 3

#define DRIVER_NAME "mychardev"

/* File operations of the driver */
static const struct file_operations mychardev_fops = 
{
    .owner   = THIS_MODULE,
    .open    = mychardev_open,
    .release = mychardev_release,
    .read    = mychardev_read,
    .write   = mychardev_write
};

/* Device data holder */
struct mychardev_device_data {
        struct cdev cdev;
};

/* global storage for device Major number */
static int mychardev_major_num = 0;

/* sysfs class structure */
static struct class *mychardev_class = NULL;

/* array of mychardev_device_data for */
static struct mychardev_device_data mychardev_data[NUM_DEV];

/* Set permissions to the character driver */
static int mychardev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_event_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int __init mychardev_init(void)
{
    dev_t device_number;

    /* Dynamically allocate a device number */
    int err = alloc_chrdev_region(&device_number, 0, NUM_DEV, DRIVER_NAME);   
    if(err < 0){
        printk(KERN_ALERT "mychardev : failed to allocate major number\n");
        return err;
    }else{
        printk(KERN_INFO "mychardev: mjor number allocated succesful\n");
    }

    /* Get a major number of device driver */
    mychardev_device_data = MAJOR(device_number);

    mychardev_class = class_create(THIS_MODULE, DRIVER_NAME);
    mychardev_class->dev_uevent = mychardev_uevent; 

    /* create sysfs class */
    mychardev_class = class_create(THIS_MODULE, DRIVER_NAME);

    /* Create necessary number of the devices */
    for(unsigned i = 0; i < NUM_DEV; i++)
    {
        /* Init new device */
        cdev_init(&mychardev_data[i].cdev, &mychardev_fops);
        mychardev_data[i].cdev.owner = THIS_MODULE;

        /* Add device to system where "i" is a Minor number of the new device */
        cdev_add(&mychardev_data[i].cdev, MKDEV(mychardev_major_num, i), 1);

        /*C reate device node /dev/mychardev-x where "x" is "i", equal to the Minor number */
        device_create(mychardev_class, NULL, MKDEV(mychardev_major_num, i), NULL, "DRIVER_NAME-%d", i);


    return 0;
}

static void __exit mychardev_cleanup(void)
{
    for(unsigned i = 0; i < NUM_DEV; i++){
        device_destroy(mychardev_class, MKDEV(mychardev_major_num, i));
    }

    class_unregister(mychardev_class);
    class_destroy(mychardev_class);

    unregister_chrdev_region(MKDEV(mychardev_major_num, 0), MINORMASK);
}

module_init(mychardev_init);
module_exit(mychardev_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("k0v4 (Kosolapov Vadim <vadkos33@outlook.com>");
MODULE_DESCRIPTION("Simple character driver");
