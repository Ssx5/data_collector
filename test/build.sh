#!/bin/bash
gcc -o modbus_test.out modbus.c  ../rio_modbus.c ../rio_config.c -I. -lmodbus -luci -lpthread
