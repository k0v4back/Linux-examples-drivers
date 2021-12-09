#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>

#include "platform.h"

int platform_driver_open(struct inode *inode, struct file *filp);
ssize_t platform_driver_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
ssize_t platform_driver_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
int platform_driver_release(struct inode *inode, struct file *filp);
loff_t platform_driver_llseek(struct file *filp, loff_t offset, int whence);
int platform_driver_probe_func(struct platform_device *);
int platform_driver_remove_func(struct platform_device *);
int check_permission(int dev_permission, int acc_mode);
struct platform_device_data * char_driver_get_platform_dt(struct device *dev);

struct cdev platform_driver_cdev;
struct class *class_platform_driver;
struct device *device_platform_driver;

enum char_device_names
{
    CHAR_DEVICE_1,
    CHAR_DEVICE_2,
    CHAR_DEVICE_3
};

/*
 * Every time when there is any match between the list of devices
 * in struct of_device_id and the device tree node, the probe function
 * of this driver will be called 
 */
struct of_device_id char_device_dt_match[] = 
{
    {.compatible = "char_device_1",.data = (void*)CHAR_DEVICE_1},
    {.compatible = "char_device_2",.data = (void*)CHAR_DEVICE_2},
    {.compatible = "char_device_3",.data = (void*)CHAR_DEVICE_3},
    { } /*Null termination*/
};

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
struct platform_driver char_platform_driver = 
{
    .probe = platform_driver_probe_func,
    .remove = platform_driver_remove_func,
    .driver = { /* Data to register information */
        .name = "char_device",
        .of_match_table = char_device_dt_match
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
    platform_driver_register(&char_platform_driver);
    pr_info("Platform driver loaded\n");

    return 0;
}

static void __exit platform_driver_cleanup(void)
{
    /* Unregister the platform driver */
    platform_driver_unregister(&char_platform_driver);

    /* Class destroy */
    class_destroy(driver_data.class_chardriver);

    /* Unregister device number for all devices */
    unregister_chrdev_region(driver_data.device_number_base, MAX_DEVICES);
    pr_info("Platform driver unloaded\n");
}

int check_permission(int dev_permission, int acc_mode)
{
    if(dev_permission == RDWR)
        return 0;

    return 0;
}

/* Check device tree */
struct platform_device_data * char_driver_get_platform_dt(struct device *dev)
{
    /* struct platform_device -> struct device -> struct device_node -> of_node  
     * of_node will consist data from dt file */
    struct device_node * dev_node = dev->of_node;
    struct platform_device_data * pdata;// temporary variable for platform data

    if(!dev_node){
        /* this probe didn't happen because of device tree node */
        return NULL;
    }

    /* Allocate memory for pdata */
    pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
    if(!pdata){
        dev_info(dev, "Cannot allocate memory \n");
        return ERR_PTR(-ENOMEM);
    }

    /* Extract propertes of the device node using dev_node 
     * and put into struct platform_device_data */

    //Extract string property (serial number)
    if(of_property_read_string(dev_node, "org,device-serial-number", &pdata->serial_number)){
        dev_info(dev, "Missing serial number property \n");
        return ERR_PTR(-EINVAL);
    }

    //Extract integer property (size)
    if(of_property_read_u32(dev_node, "org,size", &pdata->size)){
        dev_info(dev, "Missing size property \n");
        return ERR_PTR(-EINVAL);
    }
    
    //Extract integer property (permission)
    if(of_property_read_u32(dev_node, "org,perm", &pdata->permission)){
        dev_info(dev, "Missing perm property \n");
        return ERR_PTR(-EINVAL);
    }
        
    return pdata;
}

/* when the match will detected prob function will be called */
int platform_driver_probe_func(struct platform_device *pdev)
{
    int ret;

    struct device_private_data * dev_data;
    struct platform_device_data * pdata;// temporary variable for platform data
    struct device * dev = &pdev->dev;
    int char_driver_data;

    dev_info(dev, "A device is detected\n");

    /* Check device tree */
    pdata = char_driver_get_platform_dt(dev);
    if(IS_ERR(pdata))
        return PTR_ERR(pdata);

    /* Get driver data by detecting the match device */
    char_driver_data = (int)of_device_get_match_data(dev);
    
    /* Dynamically allocate memory for device private data */
    dev_data = devm_kzalloc(dev, sizeof(*dev_data), GFP_KERNEL);
    if(!dev_data){
        dev_info(dev, "Cannot allocate memory \n");
        ret = -ENOMEM;
        goto out;
    }

    /* Save the device private data pointer in platform device structure */
    dev_set_drvdata(&pdev->dev, dev_data);

    dev_data->pdata.size = pdata->size;
    dev_data->pdata.permission = pdata->permission;
    dev_data->pdata.serial_number = pdata->serial_number;

    dev_info(dev, "Device serial_number = %s\n", dev_data->pdata.serial_number);
    dev_info(dev, "Device size = %d\n", dev_data->pdata.size);
    dev_info(dev, "Device permission = %d\n", dev_data->pdata.permission);


    /* Dynamically allocate memory for device buffer using size
     * information from the platform data */
    dev_data->buffer = devm_kzalloc(&pdev->dev, dev_data->pdata.size, GFP_KERNEL);
    if(!dev_data->buffer){
        dev_info(dev, "Cannot allocate memory \n");
        ret = -ENOMEM;
        goto dev_data_free;
    }

    /* Get the device number */
    dev_data->dev_num = driver_data.device_number_base + driver_data.total_devices; 

    /* Do cdev init and cdev add */
    cdev_init(&dev_data->cdev, &platform_driver_fops); 
    dev_data->cdev.owner = THIS_MODULE;
    ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1);
    if(ret < 0){
        pr_err("Cdev add was fail\n");
        goto buffer_free;
    }

    /* Create device file for the detected platform device */
    driver_data.device_chardriver = device_create(driver_data.class_chardriver, NULL, dev_data->dev_num, NULL, "chardriver-%d", driver_data.total_devices);
    if(IS_ERR(driver_data.device_chardriver)){
        dev_info(dev, "Device create faild \n");
        goto cdev_del;
    }

    driver_data.total_devices++;

    dev_info(dev, "Probe function was successful\n");
    return 0;

    /* Error handling */
cdev_del:
    cdev_del(&dev_data->cdev);
buffer_free:
    kfree(dev_data->buffer);
dev_data_free:
    kfree(dev_data);
out:
    pr_err("Device probe failed\n");

    return ret;
}

/* Gets called when device removed from the system */
int platform_driver_remove_func(struct platform_device *pdev)
{
    /* Get device data */
    struct device_private_data *dev_data = dev_get_drvdata(&pdev->dev);

    /* Remove device that was created with device_create() */
    device_destroy(driver_data.class_chardriver, dev_data->dev_num);
    
    /* Remove cdev entry from the system */
    cdev_del(&dev_data->cdev);

    /* Free the memory held by the device */
    kfree(dev_data->buffer);
    kfree(dev_data);
    
    driver_data.total_devices--;

    dev_info(&pdev->dev, "A device is removed\n");

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

