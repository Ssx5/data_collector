
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

#include "rio_config.h"
#include "rio_thread.h"
#include "rio_modbus.h"
#include "rio_mqtt.h"
#include "rio_log.h"

typedef struct _publish_info
{
    int signal_id;
    int function_code;
    int start_address;
    int register_count;
    char publish_topic[256];
    int publish_qos;
    int publish_varied;
    int publish_period_ms;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    char last[20];
    int last_cnt;

} publish_info_t;

typedef struct _publish_config_t
{
    int count;
    publish_info_t *infos;
} publish_config_t;

typedef struct _subscribe_info
{
    char subscribe_topic[256];
    int subscribe_qos;
    int function_code;
    int start_address;
} subscribe_info_t;

typedef struct _subscribe_config_t
{
    int count;
    subscribe_info_t *infos;
} subscribe_config_t;

publish_config_t global_publish;
subscribe_config_t global_subscribe;

void load_publish_file(char *path)
{
    LOG("load_publish_file() start");

    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "open %s error\n", path);
        exit(0);
    }
#define LINE 512
    int c = 0;
    char buf[LINE];
    while (fgets(buf, LINE, fp) != NULL)
    {
        if (buf[0] != '#')
            c++;
    }

    global_publish.count = c;
    global_publish.infos = (publish_info_t *)malloc(sizeof(publish_info_t) * c);
    if (global_publish.infos == NULL)
    {
        printf("malloc error\n");
        exit(0);
    }

    fseek(fp, 0, SEEK_SET);
    int i = 0;
    while (fgets(buf, LINE, fp) != NULL)
    {
        if (buf[0] != '#')
        {
            sscanf(buf, "%d,%d,%d,%d,%d,%d,%d,%s",
                   &global_publish.infos[i].signal_id,
                   &global_publish.infos[i].function_code,
                   &global_publish.infos[i].start_address,
                   &global_publish.infos[i].register_count,
                   &global_publish.infos[i].publish_qos,
                   &global_publish.infos[i].publish_varied,
                   &global_publish.infos[i].publish_period_ms,
                   global_publish.infos[i].publish_topic);

            pthread_mutex_init(&global_publish.infos[i].mutex, NULL);
            pthread_cond_init(&global_publish.infos[i].cond, NULL);
            i++;
        }
    }

}

void load_subscribe_file(char *path)
{
    LOG("load_subscribe_file() start\n");

    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "open %s error\n", path);
        exit(0);
    }
#define LINE 512
    int c = 0;
    char buf[LINE];
    while (fgets(buf, LINE, fp) != NULL)
    {
        if (buf[0] != '#')
            c++;
    }

    global_subscribe.count = c;
    global_subscribe.infos = (subscribe_info_t *)malloc(sizeof(subscribe_info_t) * c);
    if (global_subscribe.infos == NULL)
    {
        printf("malloc error\n");
        exit(0);
    }

    fseek(fp, 0, SEEK_SET);
    int i = 0;
    while (fgets(buf, LINE, fp) != NULL)
    {
        if (buf[0] != '#')
        {
            sscanf(buf, "%d,%d,%d,%s",
                   &global_subscribe.infos[i].subscribe_qos,
                   &global_subscribe.infos[i].function_code,
                   &global_subscribe.infos[i].start_address,
                   global_subscribe.infos[i].subscribe_topic);
            i++;
        }
    }
    i = 0;
    printf("%d,%d,%d,%s\n",
           global_subscribe.infos[i].subscribe_qos,
           global_subscribe.infos[i].function_code,
           global_subscribe.infos[i].start_address,
           global_subscribe.infos[i].subscribe_topic);
}

typedef struct _publisher
{

    int publish_period_ms;
    struct timeval next_publish_time;
    pthread_cond_t *cond;

} publisher_t;

publisher_t *global_publishers;

struct timeval tm_after(struct timeval tv, int ms)
{
    tv.tv_usec += 1000 * ms;
    time_t sec = tv.tv_usec / (1000 * 1000);
    tv.tv_sec += sec;
    tv.tv_usec %= (1000 * 1000);
    return tv;
}

void publisher_init()
{
    LOG("[INFO] publisher_init() start\n");

    global_publishers = (publisher_t *)malloc(sizeof(publisher_t) * global_publish.count);
    if (global_publishers == NULL)
    {
        printf("malloc error\n");
        exit(0);
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    for (int i = 0; i < global_publish.count; ++i)
    {
        global_publishers[i].publish_period_ms = global_publish.infos[i].publish_period_ms;
        global_publishers[i].next_publish_time = tm_after(tv, global_publishers[i].publish_period_ms);
        global_publishers[i].cond = &global_publish.infos[i].cond;
    }

    LOG("[INFO] publisher_init() end\n");
}

void publish_scanner(publisher_t *p, int n)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    for (int i = 0; i < n; ++i)
    {
        if (tv.tv_sec > p[i].next_publish_time.tv_sec ||
            (tv.tv_sec == p[i].next_publish_time.tv_sec && tv.tv_usec > p[i].next_publish_time.tv_usec))
        {
            pthread_cond_broadcast(p[i].cond);
            p[i].next_publish_time = tm_after(tv, p[i].publish_period_ms);
        }
    }
}

void *publisher_scanner_routine(void *arg)
{
    LOG("[INFO] publisher_scanner_routine() start\n");
    while (1)
    {
        publish_scanner(global_publishers, global_publish.count);
        usleep(1000 * 100);
    }
    return NULL;
}
#pragma pack(1)
typedef struct _mqtt_message_t
{
    u_int64_t timestamp;
    uint8_t device_id;
    uint8_t signal_id;
    uint8_t payload[0];
} mqtt_message_t;
#pragma pack()

void *publisher_routine(void *arg)
{
    LOG("[INFO] publisher_routine() start\n");
    publish_info_t *p = (publish_info_t *)arg;

    while (1)
    {
        pthread_mutex_lock(&p->mutex);
        pthread_cond_wait(&p->cond, &p->mutex);
        pthread_mutex_unlock(&p->mutex);


        char buf[256];
        int ret = modbus_read(p->function_code, p->start_address, p->register_count, buf);
        if (p->publish_varied == 0 || p->last_cnt == 0 || p->last_cnt != ret || memcmp(p->last, buf, ret) != 0)
        {
            int size = sizeof(mqtt_message_t) + ret;
            mqtt_message_t *msg = (mqtt_message_t *)malloc(sizeof(uint8_t) * size);
            if (msg == 0)
            {
                printf("malloc error \n");
                exit(0);
            }
            struct timeval tv;
            gettimeofday(&tv, 0);
            msg->timestamp = tv.tv_sec*1000+tv.tv_usec/1000;
            msg->device_id = global_config.deviceid;
            msg->signal_id = p->signal_id;
            memcpy(msg->payload, buf, ret);
            int res = mosquitto_publish(mosq, 0, p->publish_topic, size, msg, p->publish_qos, false);
            LOG("topic: %s, size: %d, res: %d\n", p->publish_topic, size, res);
            free(msg);
            p->last_cnt = ret;
            memcpy(p->last, buf, ret);
        }
    }
    return 0;
}

#define PUBLISH_CONFIG_FILE "./config/publish.cfg"
#define SUBSCRIBE_CONFIG_FILE "./config/subscribe.cfg"

pthread_t *publish_task_init()
{
    LOG("[INFO] publish_task_init() start\n");

    load_publish_file(PUBLISH_CONFIG_FILE);

    load_subscribe_file(SUBSCRIBE_CONFIG_FILE);

    publisher_init();

    pthread_t *tid;

    tid = (pthread_t *)malloc(sizeof(pthread_t) * global_publish.count);

    if (tid == NULL)
    {
        printf("malloc error\n");
        exit(0);
    }

    for (int i = 0; i < global_publish.count; ++i)
    {
        if (pthread_create(&tid[i], NULL, publisher_routine, &global_publish.infos[i]) < 0)
        {
            printf("create thread error \n");
            exit(0);
        }
    }
    LOG("[INFO] publish_task_init() end\n");
    return tid;
}

void publish_task_wait(pthread_t *tid)
{
    LOG("[INFO] publish_task_wait() start\n");
    for (int i = 0; i < global_publish.count; ++i)
        pthread_join(tid[i], NULL);
}

pthread_t publisher_scanner_init()
{
    LOG("[INFO] publisher_scanner_init() start\n");
    pthread_t tid;
    if (pthread_create(&tid, NULL, publisher_scanner_routine, 0) < 0)
    {
        printf("create thread error \n");
        exit(0);
    }
    LOG("[INFO] publisher_scanner_init() end\n");
    return tid;
}

void publisher_scanner_wait(pthread_t tid)
{
    LOG("[INFO] publisher_scanner_wait() start\n");
    pthread_join(tid, 0);
}