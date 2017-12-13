#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <hw/spi-master.h>

int write1(int file, int device_id, unsigned char reg, unsigned char val)
{
    int res = 0;
    unsigned char wbuf[3] = {0x61, reg, val};
    
    res = spi_write(file, device_id, wbuf, 3);
    if (res < 0)
    {
        printf("Error: SPI write fail 0x%x\n", res);
        return -1; 
    }
    
    return 0;
}

int write2(int file, int device_id, unsigned char reg, unsigned char val1, unsigned char val2)
{
    int res = 0;
    unsigned char wbuf[4] = {0x61, reg, val1, val2};
    
    res = spi_write(file, device_id, wbuf, 4);
    if (res < 0)
    {
        printf("Error: SPI write fail 0x%x\n", res);
        return -1; 
    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    int res;
    int file = -1;
    spi_devinfo_t device_info;
    spi_drvinfo_t drv_info;
    int device_id;
    
    file = spi_open("/dev/spi1");
    if (file < 0)
    {
        printf ("Error: open /dev/spi1 fail\n");
        return 0;
    }
    
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

    char port_page[5] = {0x10, 0x11, 0x12, 0x13, 0x14};
    char port_master_slave[5] = {0x08, 0x08, 0x08, 0x08, 0x00};
    int i = 0;
    
    //port5 SGMII mode for J6
    write1(file, device_id, 0xFF, 0x15);
    write2(file, device_id, 0x20, 0xE0, 0x01);
    
    //dumbfwd
    write1(file, device_id, 0xFF, 0x00);
    write1(file, device_id, 0x22, 0x41);
    
    //set 89501 IMP RGMII Delay
    write1(file, device_id, 0x60, 0x1);
    
    //port0~4, master/slave select
    for (i = 0; i < 5; i++)
    {
        write1(file, device_id, 0xFF, port_page[i]);
        write1(file, device_id, 0x1c, 0x0);
        write2(file, device_id, 0x0, port_master_slave[i], 0x02);
    }
    
    printf("Polorswitch initialization done!\n");

    return 0;
}


