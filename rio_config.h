#ifndef __RIO_CONFIG_H__
#define __RIO_CONFIG_H__


typedef struct _mqtt_config_t{
    char *mqtt_addr;
    int mqtt_port;
    char *mqtt_client_id;
    char *mqtt_username;
    char *mqtt_password;
    int mqtt_clean_session;
    int mqtt_tls_enable;
    char *mqtt_tls_version;
    char *mqtt_cert_path;
    char *mqtt_cert_file;
}mqtt_config_t;

typedef struct _modbus_config_t{
    char *modbus_type;
    char *modbus_rtu_dev;
    int modbus_rtu_baudrate;
    int modbus_rtu_slaveid;

    char *modbus_tcp_addr;
    int modbus_tcp_port;
    int modbus_tcp_slaveid;

}modbus_config_t;



typedef struct _config_t{
    int deviceid;
    mqtt_config_t mqtt;
    modbus_config_t modbus;
}config_t;

extern config_t global_config;

void load_config(char *config_file);

#endif