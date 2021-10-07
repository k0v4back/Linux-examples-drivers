#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>

//Number of devices(Minor devices)
#define NUM_DEV 3

#define DRIVER_NAME "mychardriver"

/* File operations of the driver */
static const struct file_operations mychardriver_fops = 
{
    .owner   = THIS_MODULE,
    .open    = mychardriver_open,
    .release = mychardriver_release,
    .read    = mychardriver_read,
    .write   = mychardriver_write
};

/* Device data holder */
struct mychardriver_device_data {
        struct cdev cdev;
};

/* global storage for device Major number */
static int mychardriver_major_num = 0;

/* sysfs class structure */
static struct class *mychardriver_class = NULL;

/* array of mychardriver_device_data for */
static struct mychardriver_device_data mychardriver_data[NUM_DEV];

/* Set permissions to the character driver */
static int mychardriver_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_event_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int __init mychardriver_init(void)
{
    dev_t device_number;

    /* Dynamically allocate a device number */
    int err = alloc_chrdev_region(&device_number, 0, NUM_DEV, DRIVER_NAME);   
    if(err < 0){
        printk(KERN_ALERT "mychardriver : failed to allocate major number\n");
        return err;
    }else{
        printk(KERN_INFO "mychardriver: mjor number allocated succesful\n");
    }

    /* Get a major number of device driver */
    mychardriver_device_data = MAJOR(device_number);

    mychardriver_class = class_create(THIS_MODULE, DRIVER_NAME);
    mychardriver_class->dev_uevent = mychardriver_uevent; 

    /* create sysfs class */
    mychardriver_class = class_create(THIS_MODULE, DRIVER_NAME);

    /* Create necessary number of the devices */
    for(unsigned i = 0; i < NUM_DEV; i++)
    {
        /* Init new device */
        cdev_init(&mychardriver_data[i].cdev, &mychardriver_fops);
        mychardriver_data[i].cdev.owner = THIS_MODULE;

        /* Add device to system where "i" is a Minor number of the new device */
        cdev_add(&mychardriver_data[i].cdev, MKDEV(mychardriver_major_num, i), 1);

        /*C reate device node /dev/mychardev-x where "x" is "i", equal to the Minor number */
        device_create(mychardriver_class, NULL, MKDEV(mychardriver_major_num, i), NULL, "DRIVER_NAME-%d", i);


    return 0;
}

static void __exit mychardriver_cleanup(void)
{
    for(unsigned i = 0; i < NUM_DEV; i++){
        device_destroy(mychardriver_class, MKDEV(mychardriver_major_num, i));
    }

    class_unregister(mychardriver_class);
    class_destroy(mychardriver_class);

    unregister_chrdev_region(MKDEV(mychardriver_major_num, 0), MINORMASK);
}

module_init(mychardriver_init);
module_exit(mychardriver_cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("k0v4 (Kosolapov Vadim <vadkos33@outlook.com>");
MODULE_DESCRIPTION("Simple character driver");
