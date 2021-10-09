#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>

#define DEV_MEM_SIZE 512

int chardriver_open(struct inode *inode, struct file *filp);
ssize_t chardriver_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos);
ssize_t chardriver_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos);
int chardriver_release(struct inode *inode, struct file *filp);

char device_buffer[DEV_MEM_SIZE];
dev_t device_number;

struct cdev chardriver_cdev;
struct class *class_chardriver;
struct device *device_chardriver;

/* File operations of the driver */
struct file_operations chardriver_fops = 
{
    .open = chardriver_open,
    .write = chardriver_write,
    .read = chardriver_read,
    .release = chardriver_release,
    .owner = THIS_MODULE
};

static int __init chardriver_init(void)
{
    /* Dynamically allocate a device number */
    alloc_chrdev_region(&device_number, 0, 1, "chardriver"); 

    printk(KERN_INFO "%s : Device number <major>:<minor> = %d:%d \n", __func__, MAJOR(device_number), MINOR(device_number)); 

    /* Register a device cdev struct with VFS */
    cdev_init(&chardriver_cdev, &chardriver_fops);
    chardriver_cdev.owner = THIS_MODULE;
    cdev_add(&chardriver_cdev, device_number, 1);

    /* Create device class under /sys/class */
    class_chardriver = class_create(THIS_MODULE, "chardriver class");

    /*Populate the sysfs with device information */
    device_chardriver = device_create(class_chardriver, NULL, device_number, NULL, "chardriver");

    printk(KERN_INFO"Module init was successful \n");
    printk(KERN_INFO "Char driver init\n");
    return 0;
}

int chardriver_open(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "Open was successful\n");
    return 0;
}

ssize_t chardriver_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "Write requested for %zu bytes\n ", count);
    printk(KERN_INFO "Current file position: = %lld\n", *f_pos);

    /* Check the 'count' variable */
    if((*f_pos + count) > DEV_MEM_SIZE)
        count = DEV_MEM_SIZE - *f_pos;

    /* If buff is empty */
    if(!count)
        return -ENOMEM;

    /* Copy from user */
    if(copy_from_user(&device_buffer[*f_pos], buff, count))
        return -EFAULT;

    /* Update the current file position */
    *f_pos += count;

    printk(KERN_INFO "Number of bytes successfully written = %zu \n", count);
    printk(KERN_INFO "Update file position = %lld \n", *f_pos);

    /* Num of bytes which have been successfully written*/
    return count;
}

ssize_t chardriver_read(struct file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
    printk(KERN_INFO "Read requested for %zu bytes\n ", count);
    printk(KERN_INFO "Current file position: = %lld\n", *f_pos);

    /* Check the 'count' variable */
    if((*f_pos + count) > DEV_MEM_SIZE)
        count = DEV_MEM_SIZE - *f_pos;

    /* Copy to user */
    if(copy_to_user(buff, &device_buffer[*f_pos], count))
        return -EFAULT;

    /* Update the current file position */
    *f_pos += count;

    printk(KERN_INFO "Number of bytes successfully read = %zu \n", count);
    printk(KERN_INFO "Update file position = %lld \n", *f_pos);

    /* Num of bytes which have been successfully read */
    return count;
}

int chardriver_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "Close was successful\n");
    return 0;
}

static void __exit chardriver_cleanup(void)
{
    device_destroy(class_chardriver, device_number);
    class_destroy(class_chardriver);
    cdev_del(&chardriver_cdev);
    unregister_chrdev_region(device_number, 1);
    printk(KERN_INFO "Module unloaded \n");
    printk(KERN_INFO "Char driver cleanup\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kosolapov Vadim");

module_init(chardriver_init);
module_exit(chardriver_cleanup);
