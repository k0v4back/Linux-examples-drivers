#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_device.h>


int gpio_sysfs_probe(struct platform_device *);
int gpio_sysfs_remove(struct platform_device *);

/*
int platform_driver_open(struct inode *inode, struct file *filp);
ssize_t platform_driver_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
ssize_t platform_driver_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
int platform_driver_release(struct inode *inode, struct file *filp);
loff_t platform_driver_llseek(struct file *filp, loff_t offset, int whence);

struct cdev platform_driver_cdev;
struct class *class_platform_driver;
struct device *device_platform_driver;
*/

/* Device private data structure */
struct device_private_data
{
    char label[20];
};

/* Driver private data structure */
struct driver_private_data
{
    int total_devices;
    struct class *class_gpio;
};

struct driver_private_data gpio_driver_private_data;

struct of_device_id gpio_device_match[] = 
{
    {.compatible = "org,bone-gpio-sysfs"},
    {}
};

struct platform_driver gpio_sysfs_platform_driver = 
{
    .probe = gpio_sysfs_probe,
    .remove = gpio_sysfs_remove,
    .driver = {
        .name = "bone-gpio-sysfs",
        .of_match_table = of_match_ptr(gpio_device_match)
    }
};

int __init gpio_sysfs_init(void)
{
    int ret;

    gpio_driver_private_data.class_gpio = class_create(THIS_MODULE, "gpio_bone_class");
    if(IS_ERR(gpio_driver_private_data.class_gpio)){
        pr_err("Class creation failed\n");
        ret = PTR_ERR(gpio_driver_private_data.class_gpio);
        return ret; 
    }

    pr_err("Module load success\n");
    platform_driver_register(&gpio_sysfs_platform_driver);

    return 0;
}

int gpio_sysfs_probe(struct platform_device *pdev)
{
    return 0;
}

int gpio_sysfs_remove(struct platform_device *pdev)
{
    return 0;
}

void __exit gpio_sysfs_exit(void)
{
    platform_driver_unregister(&gpio_sysfs_platform_driver);
    class_destroy(gpio_driver_private_data.class_gpio);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kosolapov Vadim");

module_init(gpio_sysfs_init);
module_exit(gpio_sysfs_exit);
