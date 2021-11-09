#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include "platform.h"

int platform_driver_open(struct inode *inode, struct file *filp);
ssize_t platform_driver_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
ssize_t platform_driver_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
int platform_driver_release(struct inode *inode, struct file *filp);
loff_t platform_driver_llseek(struct file *filp, loff_t offset, int whence);
int platform_driver_probe_func(struct platform_device *);
int platform_driver_remove_func(struct platform_device *);
int check_permission(int dev_perm, int acc_mode);

struct cdev platform_driver_cdev;
struct class *class_platform_driver;
struct device *device_platform_driver;

/* Device private data structure */
struct device_private_data
{
    struct platform_device_data pdata;
    char *buffer;
    dev_t dev_num;
    struct cdev cdev;
};

/* Driver private data structure */
struct driver_private_data
{
    int total_devices;
    dev_t device_number_base;
    struct class *class_chardriver;
    struct device *device_chardriver;
};

struct driver_private_data driver_data;

/* File operations of the driver */
struct file_operations platform_driver_fops = 
{
    .open = platform_driver_open,
    .write = platform_driver_write,
    .read = platform_driver_read,
    .release = platform_driver_release,
    .llseek = platform_driver_llseek,
    .owner = THIS_MODULE
};

/* represents methods of platform_driver */
struct platform_driver platform_driver = 
{
    .probe = platform_driver_probe_func,
    .remove = platform_driver_remove_func,
    .driver = { /* Data to register information */
        .name = "char_device"
    }
};

static int __init platform_driver_init(void)
{
    int ret;

    /* Dynamically allocate a device number for all devices  */
    ret = alloc_chrdev_region(&driver_data.device_number_base,0, MAX_DEVICES, "devices"); 
    if(ret < 0){
        pr_err("Alloc chrdev faild\n");
        return ret;
    }

    /* Create device class under /sys/class  */
    driver_data.class_chardriver = class_create(THIS_MODULE, "driver_data_class");
    if(IS_ERR(driver_data.class_chardriver)){
        pr_err("Class creation failed\n");
        ret = PTR_ERR(driver_data.class_chardriver);
        unregister_chrdev_region(driver_data.device_number_base, MAX_DEVICES);
        return ret; 
    }

    /* Register a platform driver */
    platform_driver_register(&platform_driver);
    pr_info("Platform driver loaded\n");

    return 0;
}

static void __exit platform_driver_cleanup(void)
{
    /* Unregister the platform driver */
    platform_driver_unregister(&platform_driver);

    /* Class destroy */
    class_destroy(driver_data.class_chardriver);

    /* Unregister device number for all devices */
    unregister_chrdev_region(driver_data.device_number_base, MAX_DEVICES);
    pr_info("Platform driver unloaded\n");
}

int check_permission(int dev_perm, int acc_mode)
{
    if(dev_perm == RDWR)
        return 0;

    return 0;
}

/* when the match will detected prob function will be called */
int platform_driver_probe_func(struct platform_device *pdev)
{
    pr_info("A device is detected\n");
    return 0;
}

/* Gets called when device removed from the system */
int platform_driver_remove_func(struct platform_device *pdev)
{
    pr_info("A device is removed\n");
    return 0;
}

loff_t platform_driver_llseek(struct file *filp, loff_t offset, int whence)
{
    return 0;
}

int platform_driver_open(struct inode *inode, struct file *filp)
{
    pr_info("Open was successful\n");
    return 0;
}

ssize_t platform_driver_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    return -ENOMEM;
}

ssize_t platform_driver_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
    return 0;
}

int platform_driver_release(struct inode *inode, struct file *filp)
{
    pr_info("Close was successful\n");
    return 0;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kosolapov Vadim");

module_init(platform_driver_init);
module_exit(platform_driver_cleanup);

