#define RDWR 0x11
#define RDONLY 0x01
#define WRONLY 0x10

struct platform_device_data
{
    unsigned size;
    int permission;
    const char *serial_number;
};

