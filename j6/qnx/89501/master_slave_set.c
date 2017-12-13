#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <hw/spi-master.h>

void usage(char *cmd)
{
    printf("Usage: %s port master\n"
        "\tport:0~4\n"
        "\tmaster:0~1, 0 slave, 1 master\n", cmd);
}

int main(int argc, char *argv[])
{
    int res;
    int file = -1;
    spi_devinfo_t device_info;
    spi_drvinfo_t drv_info;
    int device_id;
    int port = 0;
    int master = 0;
    
    if (argc < 3)
    {
        usage(argv[0]);
        return 0;
    }
    
    port = atoi(argv[1]);
    master = atoi(argv[2]);
    
    if (port < 0 || port > 4)
    {
        usage(argv[0]);
        return 0;        
    }
    
    if (master < 0 || master > 1)
    {
        usage(argv[0]);
        return 0;        
    }
    
    file = spi_open("/dev/spi1");
    
    res = spi_getdevinfo(file, SPI_DEV_ID_NONE, &device_info);
    if (res != EOK)
    {
        printf ("Error: get deveice info fail\n");
        return 0;
    }

    //printf("the device ID %d the name: %s\n", device_info.device, device_info.name);

    device_id = device_info.device;

    res = spi_getdrvinfo(file, &drv_info );
    if (res != EOK)
    {
        printf ("Error: get driver info fail");
        return 0;
    }

    //printf("the driver version 0x%x the name: %s; feature: %d\n", drv_info.version, drv_info.name, drv_info.feature);

    char wbuf[4] = {0x0};
    char port_page[5] = {0x10, 0x11, 0x12, 0x13, 0x14};
    char port_master_slave[2] = {0x00, 0x08};
    int i = 0;
    
    wbuf[0] = 0x61;
    wbuf[1] = 0xFF;
    wbuf[2] = port_page[port];
    res = spi_write(file, device_id, wbuf, 3);
    if (res < 0)
    {
        printf ("Error: SPI write fail 0x%x\n", res);
        return 0; 
    }
    
    usleep(100);
    
    wbuf[0] = 0x61;
    wbuf[1] = 0x1c;
    wbuf[2] = 0x0;
    
    res = spi_write(file, device_id, wbuf, 3);
    if (res < 0)
    {
        printf ("Error: SPI write fail 0x%x\n", res);
        return 0; 
    }
    
    usleep(100);
    
    wbuf[0] = 0x61;
    wbuf[1] = 0x0;
    wbuf[2] = port_master_slave[master];
    wbuf[3] = 0x02;
    
    res = spi_write(file, device_id, wbuf, 4);
    if (res < 0)
    {
        printf ("Error: SPI write fail 0x%x\n", res);
        return 0; 
    }
    
    printf("SPI finish\n");

    return 0;
}
