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
#include <linux/gpio/consumer.h>

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
    struct gpio_desc *desc;
};

/* Driver private data structure */
struct driver_private_data
{
    int total_devices;
    struct class *class_gpio;
};

/* Create device attributes */
ssize_t direction_show(struct device *dev, struct device_attribute *attr, \
                        char *buf)
{
    return 0;
}

ssize_t direction_store(struct device *dev, struct device_attribute *attr, \
                        const char *buf, size_t count)
{
    return 0;
}

ssize_t value_show(struct device *dev, struct device_attribute *attr, \
                        char *buf)
{
    return 0;
}

ssize_t value_store(struct device *dev, struct device_attribute *attr, \
                        const char *buf, size_t count)
{
    return 0;
}

ssize_t label_show(struct device *dev, struct device_attribute *attr, \
                        char *buf)
{
    return 0;
}

static DEVICE_ATTR_RW(direction);
static DEVICE_ATTR_RW(value);
static DEVICE_ATTR_RO(label);

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
    const char *name;
    int i = 0;
    int ret;

    struct device *dev = &pdev->dev;

    /* Parent device node */
    struct device_node *parent = pdev->dev.of_node;

    /* Child device node */
    struct device_node *child = NULL;

    struct device_private_data *device_data;

    for_each_available_child_of_node(parent, child)
    {
        /* Allocate memory for device private data */
        device_data = devm_kzalloc(dev, sizeof(*device_data), GFP_KERNEL);
        if(!device_data){
            dev_err(dev, "Cannot allocate memory\n");
            return -ENOMEM;
        }

        /* Fill with data device private data structer from device tree node */
        if(of_property_read_string(child, "label", &name)){
            dev_warn(dev, "Missing lable information \n");
            snprintf(device_data->label, sizeof(device_data->label), "unkngpio%d", i);
        }else{
            strcpy(device_data->label, name);
            dev_info(dev, "GPIO label = %s\n", device_data->label);
        }

        /* Get GPIO as a reference to GPIO descriptor */
        device_data->desc = devm_fwnode_get_gpiod_from_child(dev, "bone", &child->fwnode, \
                                                           GPIOD_ASIS, device_data->label);
        if(IS_ERR(device_data->desc)){
            ret = PTR_ERR(device_data->desc); //Extract error
            if(ret == -ENOENT)
                dev_err(dev,"No GPIO has been assigned to the requested function and/or index\n");
            return ret;
        }

        /* Set the gpio direction to output */
        ret = gpiod_direction_output(device_data->desc, 0);
        if(ret){
            dev_err(dev, "gpio direction set failed \n");
            return ret;
        }

        i++; 
    } 

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
