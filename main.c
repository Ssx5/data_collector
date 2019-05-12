#include <stdio.h>
#include "rio_config.h"
#include "rio_mqtt.h"
#include "rio_modbus.h"
#include "rio_thread.h"
#include "rio_log.h"

#define CONFIG_FILE "./config/remoteio"


int main()
{
    LOG("main() start\n");
    load_config(CONFIG_FILE);
    mqtt_init(&mosq);
#ifndef DEBUG
    modbus_init(&modbus_ctx);
#endif
    pthread_t *tids = publish_task_init();
    pthread_t tid = publisher_scanner_init();

    /*
        这里可以执行任意过程
    */
    publisher_scanner_wait(tid);
    publish_task_wait(tids);
    /*
        Never Run Here !
    */
    return 0;
}