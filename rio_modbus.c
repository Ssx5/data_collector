#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "rio_modbus.h"
#include "rio_config.h"

modbus_t *modbus_ctx;

void modbus_init(modbus_t **ctx)
{
    printf("modbus_type: %s\n", global_config.modbus.modbus_type);
    printf("modbus_rtu_dev: %s\n", global_config.modbus.modbus_rtu_dev);
    printf("modbus_rtu_baudrate: %d\n", global_config.modbus.modbus_rtu_baudrate);
    printf("modbus_rtu_slaveid: %d\n", global_config.modbus.modbus_rtu_slaveid);
    
    if(strcmp(global_config.modbus.modbus_type, "modbus_tcp") == 0)
        *ctx = modbus_new_tcp(global_config.modbus.modbus_tcp_addr, global_config.modbus.modbus_tcp_port);
    else
        *ctx = modbus_new_rtu(global_config.modbus.modbus_rtu_dev, global_config.modbus.modbus_rtu_baudrate, 'N', 8, 1);
    
    if (*ctx == NULL)
    {
        fprintf(stderr, "modbus_new_tcp error\n");
        exit(0);
    }

    if(strcmp(global_config.modbus.modbus_type, "modbus_tcp") == 0)
        modbus_set_slave(*ctx, global_config.modbus.modbus_tcp_slaveid);
    else
        modbus_set_slave(*ctx, global_config.modbus.modbus_rtu_slaveid);

    if (modbus_connect(*ctx) == -1)
    {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(*ctx);
        exit(0);
    }
    fprintf(stdout, "modbus_connect() success!\n");

    modbus_set_response_timeout(*ctx, 1, 0);
}


#ifdef DEBUG

int r_debug = 0;

int modbus_read(int function_code, int start_address, int register_count, char* buf)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("%d.%.6dfunction_code: %d, start_address: %d, register_count:%d\n", tv.tv_sec, tv.tv_usec,
        function_code, start_address, register_count);
    char recv[] = {1,2,3,4,5};
    recv[0] = r_debug / 3;
    r_debug++;
    memcpy(buf, recv, sizeof(recv));
    usleep(50 * 1000);
    return sizeof(recv);
}
#else
int modbus_read(int function_code, int start_address, int register_count, char* buf)
{
    
    uint8_t* bitbuf = 0;
    uint16_t* regbuf = 0;
    int ret;
    switch (function_code)
        {
        case 1:
            bitbuf = (uint8_t*)malloc(sizeof(uint8_t) * register_count);
            ret = modbus_read_bits(modbus_ctx, start_address, register_count, bitbuf);
            if (ret < 0)
            {
                fprintf(stderr, "[Modbus]:%s\n", modbus_strerror(errno));
                return 0;
            }
            break;
        case 2:
            bitbuf = (uint8_t*)malloc(sizeof(uint8_t) * register_count);
            ret = modbus_read_input_bits(modbus_ctx, start_address, register_count, bitbuf);
            if (ret < 0)
            {
                fprintf(stderr, "[Modbus]: %s\n", modbus_strerror(errno));
                return 0;
            }
            break;
        case 3:
            regbuf = (uint16_t *)malloc(sizeof(uint16_t) * register_count);
            ret = modbus_read_registers(modbus_ctx, start_address, register_count, regbuf);
            if (ret < 0)
            {
                fprintf(stderr, "[Modbus]: %s\n", modbus_strerror(errno));
                return 0;
            }
            ret *= 2;
            break;
        case 4:
            regbuf = (uint16_t *)malloc(sizeof(uint16_t) * register_count);
            ret = modbus_read_input_registers(modbus_ctx, start_address, register_count, regbuf);
            if (ret < 0)
            {
                fprintf(stderr, "[Modbus]: %s\n", modbus_strerror(errno));
                return 0;
            }
            ret *= 2;
            break;
        default:
            fprintf(stderr, "[Modbus]: %s\n", "Unknown function_code");
            return 0;
            break;
        }

    if(bitbuf)
    {
        memcpy(buf, bitbuf, ret);
        free(bitbuf);
        return ret;
    }
    if(regbuf)
    {
        memcpy(buf, regbuf, ret);
        free(regbuf);
        return ret;
    }
}

#endif


