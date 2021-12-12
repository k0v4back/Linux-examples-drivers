#ifndef PLATFORM_DRIVER_DT_SYSFS_H
#define PLATFORM_DRIVER_DT_SYSFS_H

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#include <linux/platform_device.h>
#include<linux/slab.h>
#include<linux/mod_devicetable.h>
#include<linux/of.h>
#include<linux/of_device.h>

#include "platform.h"

int platform_driver_open(struct inode *inode, struct file *filp);
ssize_t platform_driver_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
ssize_t platform_driver_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
int platform_driver_release(struct inode *inode, struct file *filp);
loff_t platform_driver_llseek(struct file *filp, loff_t offset, int whence);
int platform_driver_probe_func(struct platform_device *);
int platform_driver_remove_func(struct platform_device *);
int check_permission(int dev_perm, int acc_mode);
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

#endif
