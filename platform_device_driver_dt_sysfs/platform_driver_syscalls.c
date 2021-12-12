#include "platform_driver_dt_sysfs.h"

int check_permission(int dev_perm, int acc_mode)
{
    if(dev_perm == RDWR)
        return 0;

    //ensures readonly access
    if( (dev_perm == RDONLY) && ( (acc_mode & FMODE_READ) && !(acc_mode & FMODE_WRITE) ) )
        return 0;
    
    //ensures writeonly access
    if( (dev_perm == WRONLY) && ( (acc_mode & FMODE_WRITE) && !(acc_mode & FMODE_READ) ) )
        return 0;
    
    return -EPERM;
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

