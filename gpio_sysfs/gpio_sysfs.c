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
    struct device **dev;
};

struct driver_private_data gpio_driver_private_data;

/* Create device attributes */
ssize_t direction_show(struct device *dev, struct device_attribute *attr, \
                        char *buf)
{
    struct device_private_data *device_data = dev_get_drvdata(dev);

    char *p_direction = NULL;
    int direction = gpiod_get_direction(device_data->desc);

    if(direction < 0)
        return direction;
    
    /* if dir = 0 , then show "out". if dir =1 , then show "in" */
    p_direction = (direction == 0) ? "out":"in";

    return sprintf(buf, "%s\n", p_direction);
}

ssize_t direction_store(struct device *dev, struct device_attribute *attr, \
                        const char *buf, size_t count)
{
    int ret;
    struct device_private_data *device_data = dev_get_drvdata(dev);

    if(sysfs_streq(buf, "in") )
        ret = gpiod_direction_input(device_data->desc);
    else if (sysfs_streq(buf, "out"))
        ret = gpiod_direction_output(device_data->desc, 0);
    else
        ret = -EINVAL;

    return ret ? : count;

    return 0;
}

ssize_t value_show(struct device *dev, struct device_attribute *attr, \
                        char *buf)
{
    struct device_private_data *device_data = dev_get_drvdata(dev);
    int value = gpiod_get_value(device_data->desc);

    return sprintf(buf,"%d\n",value);
}

ssize_t value_store(struct device *dev, struct device_attribute *attr, \
                        const char *buf, size_t count)
{
    struct device_private_data *device_data = dev_get_drvdata(dev);
    int ret;
    long value;
    
    ret = kstrtol(buf, 0, &value);
    if(ret)
        return ret;    
    gpiod_set_value(device_data->desc, value);

    return count;
}

ssize_t label_show(struct device *dev, struct device_attribute *attr, \
                        char *buf)
{
    struct device_private_data *device_data = dev_get_drvdata(dev);
    return sprintf(buf, "%s\n", device_data->label);
}

static DEVICE_ATTR_RW(direction);
static DEVICE_ATTR_RW(value);
static DEVICE_ATTR_RO(label);

static struct attribute *gpio_attrs[] = 
{
    &dev_attr_direction.attr,
    &dev_attr_value.attr,
    &dev_attr_label.attr,
    NULL
};

static struct attribute_group gpio_attr_group =
{
    .attrs = gpio_attrs
};

static const struct attribute_group *gpio_attr_groups[] = 
{
    &gpio_attr_group,
    NULL
};

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

    gpio_driver_private_data.total_devices = of_get_child_count(parent);
    if(!gpio_driver_private_data.total_devices){
        dev_err(dev, "No devices found\n");
        return -EINVAL;
    }

    dev_info(dev, "Total devices found = %d\n", gpio_driver_private_data.total_devices);

    gpio_driver_private_data.dev = devm_kzalloc(dev, sizeof(struct device *) * \
            gpio_driver_private_data.total_devices , GFP_KERNEL);

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

        /* Create devices under /sys/class/bone_gpios */
        gpio_driver_private_data.dev[i] = device_create_with_groups(gpio_driver_private_data.class_gpio, \
                dev, 0, device_data, gpio_attr_groups, device_data->label);
        if(IS_ERR(gpio_driver_private_data.dev[i])){
            dev_err(dev, "Error in device_create \n");
            return PTR_ERR(gpio_driver_private_data.dev[i]);
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
