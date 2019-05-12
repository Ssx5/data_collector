#ifndef __RIO_MODBUS_H__
#define __RIO_MODBUS_H__

#include <modbus/modbus.h>


void modbus_init(modbus_t **ctx);
int modbus_read(int function_code, int start_address, int register_count, char* buf);
extern modbus_t *modbus_ctx;
#endif