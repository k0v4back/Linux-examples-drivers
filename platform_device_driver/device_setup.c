#include <linux/module.h>
#include <linux/platform_device.h>

#include "platform.h"


/* Call back to free the device memory after all work */
void device_release(struct device *dev)
{
    pr_info("Device released \n");
}


/* create a platform device data */

struct platform_device_data platform_data[3] = 
{
    [0] = {
        .size = 256,
        .permission = RDWR,
        .serial_number = "PLSF343"
    },
    [1] = {
        .size = 512,
        .permission = RDWR,
        .serial_number = "DFWE652"
    },
    [2] = {
        .size = 1024,
        .permission = RDONLY,
        .serial_number = "LPID912"
    }
};


/* create 3 platfrom devices */

struct platform_device platform_device_1 = 
{
    .name = "char_device",
    .id = 0,
    .dev = { /* send the data to the platform device when the matching occurs */
        .platform_data = &platform_data[0],
        .release = device_release
    }
};

struct platform_device platform_device_2 = 
{
    .name = "char_device",
    .id = 1,
    .dev = { /* send the data to the platform device when the matching occurs */
        .platform_data = &platform_data[1],
        .release = device_release
    }
};

struct platform_device platform_device_3 = 
{
    .name = "char_device",
    .id = 2,
    .dev = { /* send the data to the platform device when the matching occurs */
        .platform_data = &platform_data[2],
        .release = device_release
    }
};


static int __init platform_device_init(void)
{
    platform_device_register(&platform_device_1); 
    platform_device_register(&platform_device_2); 
    platform_device_register(&platform_device_3); 

    pr_info("Platform device module loaded");

    return 0;
}

static void __exit platform_device_exit(void)
{
    platform_device_unregister(&platform_device_1);
    platform_device_unregister(&platform_device_2);
    platform_device_unregister(&platform_device_3);

    pr_info("Platform device module unloaded");
}

module_init(platform_device_init);
module_exit(platform_device_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module that registers platform devices");
