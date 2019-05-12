#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>


#include <../rio_modbus.h>
#include <../rio_config.h>

#define CONFIG_FILE "/etc/config/remoteio"


void *modbus_read_routine(void *arg);
void *modbus_write_routine(void* arg);
void * modbus_scanner(void * arg);

pthread_mutex_t modbus_read_lock;
pthread_mutex_t modbus_write_lock;
pthread_cond_t modbus_read_cond;
pthread_cond_t modbus_write_cond;


int main(int argc, char *argv[])
{
    load_config(CONFIG_FILE);
    modbus_init(&modbus_ctx);
    
    pthread_mutex_init(&modbus_read_lock, 0);
    pthread_cond_init(&modbus_read_cond, 0);
    pthread_mutex_init(&modbus_write_lock, 0);
    pthread_cond_init(&modbus_write_cond, 0);
    
    
    
    pthread_t read_tid;
    pthread_t write_tid;
    pthread_t scan_tid;
    pthread_create(&scan_tid, 0, modbus_scanner, 0);
    pthread_create(&read_tid, 0, modbus_read_routine, 0);
    pthread_create(&write_tid, 0, modbus_write_routine, 0);

    pthread_join(write_tid, 0);
    pthread_join(read_tid, 0);
    pthread_join(scan_tid, 0);

    return 0;
}

int g_i = 0;
void * modbus_scanner(void * arg)
{
    while(1)
    {
        if(g_i %2 == 0)
        {
            pthread_cond_broadcast(&modbus_read_cond);
        }
        else
        {
            pthread_cond_broadcast(&modbus_write_cond);
        }
        g_i++;
        usleep(50 * 1000);
    }

}

void *modbus_read_routine(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&modbus_read_lock);
        pthread_cond_wait(&modbus_read_cond, &modbus_read_lock);
        pthread_mutex_unlock(&modbus_read_lock);

        int start_address = 0;
        int register_count = 16;
        uint16_t buf[32];
        int ret = modbus_read_registers(modbus_ctx, start_address, register_count, buf);
        if (ret < 0)
        {
            fprintf(stderr, "[Modbus Read]: %s\n", modbus_strerror(errno));
            return 0;
        }
        for (int i = 0; i < ret; ++i)
        {
            printf("%.2X ", buf[i]);
        }
        printf("\n");

    }
}


int g_j = 0;

void *modbus_write_routine(void* arg)
{
    while(1)
    {
        pthread_mutex_lock(&modbus_write_lock);
        pthread_cond_wait(&modbus_write_cond, &modbus_write_lock);
        pthread_mutex_unlock(&modbus_write_lock);

        uint16_t buf[2];
        if(g_i %2 == 0)
        {
            buf[0] = 1;
            buf[1] = 1;
        }
        else
        {
            buf[0] = 0;
            buf[1] = 0;
        }
        int ret  = modbus_write_registers(modbus_ctx, 2, 2, buf);
        if (ret < 0)
        {
            fprintf(stderr, "[Modbus Write]: %s\n", modbus_strerror(errno));
            return 0;
        }

    }
    return 0;
}