#include <stdlib.h>
#include <uci.h>
#include <string.h>

#include "rio_config.h"
#include "rio_log.h"
config_t global_config;


void load_config(char *config_file)
{
    LOG("load_config(%s) start\n", config_file);
    struct uci_context *ctx;
    struct uci_package *pkg = NULL;
    struct uci_element *e;
    char *tmp;
    const char *value;

    ctx = uci_alloc_context(); // 申请一个UCI上下文.
    if (UCI_OK != uci_load(ctx, config_file, &pkg))
        goto cleanup; //如果打开UCI文件失败,则跳到末尾 清理 UCI 上下文.

    /*遍历UCI的每一个节*/
    uci_foreach_element(&pkg->sections, e)
    {
        struct uci_section *s = uci_to_section(e);
        printf("section s's type is %s.\n", s->type);
        if (!strcmp("remoteio", s->type))
        {
            if (NULL != (value = uci_lookup_option_string(ctx, s, "device_id")))
            {
                tmp = strdup(value);
                global_config.deviceid = atoi(tmp);
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "mqtt_addr")))
            {
                tmp = strdup(value); 
                //printf("%s's num_attr is %s.\n", s->e.name, value);
                global_config.mqtt.mqtt_addr = tmp;
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "mqtt_port")))
            {
                tmp = strdup(value);
                global_config.mqtt.mqtt_port = atoi(tmp);
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "mqtt_client_id")))
            {
                tmp = strdup(value);
                global_config.mqtt.mqtt_client_id = tmp;
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "mqtt_username")))
            {
                tmp = strdup(value);
                global_config.mqtt.mqtt_username = tmp;
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "mqtt_password")))
            {
                tmp = strdup(value);
                global_config.mqtt.mqtt_password = tmp;
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "mqtt_clean_session")))
            {
                tmp = strdup(value);
                global_config.mqtt.mqtt_clean_session = atoi(tmp);
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "mqtt_tls_enable")))
            {
                tmp = strdup(value);
                global_config.mqtt.mqtt_tls_enable = atoi(tmp);
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "mqtt_tls_version")))
            {
                tmp = strdup(value);
                global_config.mqtt.mqtt_tls_version = tmp;
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "mqtt_cert_path")))
            {
                tmp = strdup(value);
                global_config.mqtt.mqtt_cert_path = tmp;
            }

            if (NULL != (value = uci_lookup_option_string(ctx, s, "modbus_type")))
            {
                tmp = strdup(value);
                global_config.modbus.modbus_type = tmp;
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "modbus_rtu_serial")))
            {
                tmp = strdup(value);
                global_config.modbus.modbus_rtu_dev = tmp;
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "modbus_rtu_baudrate")))
            {
                tmp = strdup(value);
                global_config.modbus.modbus_rtu_baudrate = atoi(tmp);
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "modbus_rtu_slaveid")))
            {
                tmp = strdup(value);
                global_config.modbus.modbus_rtu_slaveid = atoi(tmp);
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "modbus_tcp_addr")))
            {
                tmp = strdup(value);
                global_config.modbus.modbus_tcp_addr = tmp;
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "modbus_tcp_port")))
            {
                tmp = strdup(value);
                global_config.modbus.modbus_tcp_port = atoi(tmp);
            }
            if (NULL != (value = uci_lookup_option_string(ctx, s, "modbus_tcp_slaveid")))
            {
                tmp = strdup(value);
                global_config.modbus.modbus_tcp_slaveid = atoi(tmp);
            }
        }
    }
    uci_unload(ctx, pkg);
cleanup:
    uci_free_context(ctx);
    ctx = NULL;
}