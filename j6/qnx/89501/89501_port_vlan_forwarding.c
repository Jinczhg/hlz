#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <hw/spi-master.h>

void usage(char *cmd)
{
    printf("Usage: %s port mask\n"
        "\tport:0~8, but 6 is Reserved\n"
        "\tmask:0x0~0x1FF\n", cmd);
}

int main(int argc, char *argv[])
{
    int res;
    int file = -1;
    spi_devinfo_t device_info;
    spi_drvinfo_t drv_info;
    int device_id;
    int port = 0;
    int mask = 0;
    
    if (argc < 3)
    {
        usage(argv[0]);
        return 0;
    }
    
    //port = atoi(argv[1]);
    //mask = atoi(argv[2]);
    sscanf(argv[1], "%x", &port);
    sscanf(argv[2], "%x", &mask);
    
    if (port < 0 || port > 8)
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

    printf("port%d, forwarding mask: %X\n", port, mask & 0x1FF);

    char wbuf[4] = {0x0};
    char port_offset[9] = {0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0x10};
    char port_master_slave[2] = {0x00, 0x08};
    int i = 0;
    
    wbuf[0] = 0x61;
    wbuf[1] = 0xFF;
    wbuf[2] = 0x31;
    res = spi_write(file, device_id, wbuf, 3);
    if (res < 0)
    {
        printf ("Error: SPI write page fail 0x%x\n", res);
        return 0; 
    }
    
    usleep(100);
    
    wbuf[0] = 0x61;
    wbuf[1] = port_offset[port];
    wbuf[2] = mask & 0xFF;
    wbuf[3] = (mask >> 8) & 0x1;
    
    res = spi_write(file, device_id, wbuf, 4);
    if (res < 0)
    {
        printf ("Error: SPI write offset fail 0x%x\n", res);
        return 0; 
    }
    
    printf("SPI finish\n");

    return 0;
}
