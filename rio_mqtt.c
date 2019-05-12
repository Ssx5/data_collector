#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "rio_mqtt.h"
#include "rio_config.h"


#define MQTT_CERT_PATH "./config"
#define MQTT_CERT_FILE "./config/digicert.cer"

struct mosquitto *mosq;


void connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
    if (!result)
    {
        fprintf(stdout, "Connect success!\n");
    }
    else
    {
        fprintf(stderr, "Connect failed: %d\n", errno);
        exit(0);
    }
}


void mqtt_init(struct mosquitto **mosq)
{
    int ret;
    mosquitto_lib_init();

    int major, minor, revision;
    mosquitto_lib_version(&major, &minor, &revision);
    printf("libmosquitto: major: %d, minor: %d, revision: %d\n", major, minor, revision);

    *mosq = mosquitto_new(global_config.mqtt.mqtt_client_id, global_config.mqtt.mqtt_clean_session, NULL);

    if (!mosq)
    {
        fprintf(stderr, "mosquitto_new() error\n");
        exit(0);
    }

    if (global_config.mqtt.mqtt_username && global_config.mqtt.mqtt_password
    &&(strlen(global_config.mqtt.mqtt_username) > 0 || strlen(global_config.mqtt.mqtt_password) > 0))
        mosquitto_username_pw_set(*mosq, global_config.mqtt.mqtt_username, global_config.mqtt.mqtt_password);

    if (global_config.mqtt.mqtt_tls_enable)
    {
        if(global_config.mqtt.mqtt_cert_path == 0 || global_config.mqtt.mqtt_cert_file == 0)
        {
            global_config.mqtt.mqtt_cert_path = MQTT_CERT_PATH;
            global_config.mqtt.mqtt_cert_file = MQTT_CERT_FILE;
        }
        ret = mosquitto_tls_set(*mosq, global_config.mqtt.mqtt_cert_file,
                                global_config.mqtt.mqtt_cert_path,
                                NULL,
                                NULL,
                                NULL);

        if (ret != MOSQ_ERR_SUCCESS)
        {
            fprintf(stderr, "mosquitto_tls_set() error: %d, file: %s, path: %s\n", ret,
                global_config.mqtt.mqtt_cert_file, global_config.mqtt.mqtt_cert_path);
            exit(0);
        }

        ret = mosquitto_tls_insecure_set(*mosq, false);
        if (ret != MOSQ_ERR_SUCCESS)
        {
            fprintf(stderr, "mosquitto_tls_insecure_set() error: %d\n", ret);
            exit(0);
        }

        ret = mosquitto_tls_opts_set(*mosq, 1, global_config.mqtt.mqtt_tls_version, NULL);
        if (ret != MOSQ_ERR_SUCCESS)
        {
            fprintf(stderr, "mosquitto_tls_opts_set() error: %d\n", ret);
            exit(0);
        }
    }
    //mosquitto_message_callback_set(*mosq, message_callback);
    mosquitto_connect_callback_set(*mosq, connect_callback);
    //mosquitto_subscribe_callback_set(*mosq, subscribe_callback);
    //mosquitto_publish_callback_set(*mosq, publish_callback);

    ret = mosquitto_connect(*mosq, global_config.mqtt.mqtt_addr, global_config.mqtt.mqtt_port, 600);
    if (ret != MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto_connect %s:%s error %d, errno: %d\n",global_config.mqtt.mqtt_addr,global_config.mqtt.mqtt_port, ret, errno);
        exit(0);
    }

    int loop = mosquitto_loop_start(*mosq);
    if(loop != MOSQ_ERR_SUCCESS)
    {
        printf("mosquitto loop error\n");
        exit(0);
    }
}