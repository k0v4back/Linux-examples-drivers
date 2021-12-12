#include "platform_driver_dt_sysfs.h"

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

ssize_t show_serial_num(struct device *dev, struct device_attribute *attr,char *buf)
{
    /* get access to the device private data */
    struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);

    return sprintf(buf,"%s\n",dev_data->pdata.serial_number);

}

ssize_t show_max_size(struct device *dev, struct device_attribute *attr,char *buf)
{
    /* get access to the device private data */
    struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);

    return sprintf(buf,"%d\n",dev_data->pdata.size);

}

ssize_t store_max_size(struct device *dev, struct device_attribute *attr,const char *buf, size_t count)
{
    long result;
    int ret;
    struct pcdev_private_data *dev_data = dev_get_drvdata(dev->parent);

    ret = kstrtol(buf,10,&result);
    if(ret)
        return ret;

    dev_data->pdata.size = result;

    dev_data->buffer = krealloc(dev_data->buffer,dev_data->pdata.size,GFP_KERNEL);

    return count;
}

/*create 2 variables of struct device_attribute */
static DEVICE_ATTR(max_size, S_IRUGO|S_IWUSR, show_max_size, store_max_size);
static DEVICE_ATTR(serial_num, S_IRUGO, show_serial_num, NULL);

int platform_driver_sysfs_create_files(struct device *pcd_dev)
{
    int ret;
#if 0
    ret = sysfs_create_file(&pcd_dev->kobj,&dev_attr_max_size.attr);
    if(ret)
        return ret;
    return sysfs_create_file(&pcd_dev->kobj,&dev_attr_serial_num.attr);
#endif 
    return sysfs_create_group(&pcd_dev->kobj, &pcd_attr_group);
}

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

    /* Create attribute file */
    ret = platform_driver_sysfs_create_files(pcdrv_data.device_pcd);
    if(ret){
        device_destroy(pcdrv_data.class_pcd,dev_data->dev_num);
        return ret;
    }

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

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kosolapov Vadim");

module_init(platform_driver_init);
module_exit(platform_driver_cleanup);

