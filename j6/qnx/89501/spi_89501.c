#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <hw/spi-master.h>

int main(int argc, char *argv[])
{
    int res;
    int file = -1;
    spi_devinfo_t device_info;
    spi_drvinfo_t drv_info;
    int device_id;
    
    file = spi_open("/dev/spi1");
    
    res = spi_getdevinfo(file, SPI_DEV_ID_NONE, &device_info);
    if (res != EOK)
    {
        printf ("Error: get deveice info fail\n");
        return 0;
    }

    printf("the device ID %d the name: %s\n", device_info.device, device_info.name);

    device_id = device_info.device;

    res = spi_getdrvinfo(file, &drv_info );
    if (res != EOK)
    {
        printf ("Error: get driver info fail");
        return 0;
    }

    printf("the driver version 0x%x the name: %s; feature: %d\n", drv_info.version, drv_info.name, drv_info.feature);

    char wbuf[4] = {0x0};
    char port_page[5] = {0x10, 0x11, 0x12, 0x13, 0x14};
    char port_master_slave[5] = {0x08, 0x08, 0x08, 0x08, 0x00};
    int i = 0;
    
    for (i = 0; i < 5; i++)
    {
        wbuf[0] = 0x61;
        wbuf[1] = 0xFF;
        wbuf[2] = port_page[i];
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
        wbuf[2] = port_master_slave[i];
        wbuf[3] = 0x02;
    
        res = spi_write(file, device_id, wbuf, 4);
        if (res < 0)
        {
            printf ("Error: SPI write fail 0x%x\n", res);
            return 0; 
        }
    
        usleep(100);
    }
    
    printf("SPI finish\n");

    return 0;
}


