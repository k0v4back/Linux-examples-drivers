#include <linux/module.h>
#include <linux/platform_device.h>

/* create 3 platfrom devices */

struct platform_device platform_device_1 = 
{
    .name = "char_device",
    .id = 0
};

struct platform_device platform_device_2 = 
{
    .name = "char_device",
    .id = 1
};

struct platform_device platform_device_3 = 
{
    .name = "char_device",
    .id = 2
};


static int __init platform_device_init(void)
{
    platform_device_register(&platform_device_1); 
    platform_device_register(&platform_device_2); 
    platform_device_register(&platform_device_3); 

    pr_info("Platform device init was successful");

    return 0;
}

static void __exit platform_device_exit(void)
{
    platform_device_unregister(&platform_device_1);
    platform_device_unregister(&platform_device_2);
    platform_device_unregister(&platform_device_3);

    pr_info("Platform device exit was successful");
}

module_init(platform_device_init);
module_exit(platform_device_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module that registers platform devices");
