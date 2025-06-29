#pragma once
#ifndef _DEVICE_SERIAL_H
#define _DEVICE_SERIAL_H


#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)


#define DEVICE_SERIAL_NUM	10001
#define DEVICE_SERIAL_STR	STR(DEVICE_SERIAL_NUM)


#endif

