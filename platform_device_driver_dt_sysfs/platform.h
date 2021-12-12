#ifndef PLATFORM_H_
#define PLATFORM_H_

#define RDWR 0x11
#define RDONLY 0x01
#define WRONLY 0x10

#define MAX_DEVICES 10

struct platform_device_data
{
    unsigned size;
    int permission;
    const char *serial_number;
};

#endif
