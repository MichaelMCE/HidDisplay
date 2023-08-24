#pragma once
#ifndef _FT5216_H_
#define _FT5216_H_


#define FT5216_DEVICE_MODE                0x00
#define FT5216_GESTURE_ID                 0x01
#define FT5216_TOUCH_POINTS               0x02

#define FT5216_TOUCH1_EV_FLAG             0x03
#define FT5216_TOUCH1_XH                  0x03
#define FT5216_TOUCH1_XL                  0x04
#define FT5216_TOUCH1_YH                  0x05
#define FT5216_TOUCH1_YL                  0x06

#define FT5216_TOUCH2_EV_FLAG             0x09
#define FT5216_TOUCH2_XH                  0x09
#define FT5216_TOUCH2_XL                  0x0A
#define FT5216_TOUCH2_YH                  0x0B
#define FT5216_TOUCH2_YL                  0x0C

#define FT5216_TOUCH3_EV_FLAG             0x0F
#define FT5216_TOUCH3_XH                  0x0F
#define FT5216_TOUCH3_XL                  0x10
#define FT5216_TOUCH3_YH                  0x11
#define FT5216_TOUCH3_YL                  0x12

#define FT5216_TOUCH4_EV_FLAG             0x15
#define FT5216_TOUCH4_XH                  0x15
#define FT5216_TOUCH4_XL                  0x16
#define FT5216_TOUCH4_YH                  0x17
#define FT5216_TOUCH4_YL                  0x18

#define FT5216_TOUCH5_EV_FLAG             0x1B
#define FT5216_TOUCH5_XH                  0x1B
#define FT5216_TOUCH5_XL                  0x1C
#define FT5216_TOUCH5_YH                  0x1D
#define FT5216_TOUCH5_YL                  0x1E

#define FT5216_REG_THGROUP                0x80   /* touch threshold related to sensitivity */
#define FT5216_REG_THPEAK                 0X81
#define FT5216_REG_THCAL                  0X82
#define FT5216_REG_THWATER                0X83
#define FT5216_REG_THTEMP                 0X84
#define FT5216_REG_THDIFF                 0X85
#define FT5216_REG_CTRL                   0X86
#define FT5216_REG_TIMEENTERMONITOR       0X87
#define FT5216_REG_PERIODACTIVE           0X88   /* report rate */
#define FT5216_REG_PERIODMONITOR          0X89
#define FT5216_REG_HEIGHT_B               0X8A
#define FT5216_REG_MAX_FRAME              0X8B
#define FT5216_REG_DIST_MOVE              0X8C
#define FT5216_REG_DIST_POINT             0X8D
#define FT5216_REG_FEG_FRAME              0X8E
#define FT5216_REG_SINGLE_CLICK_OFFSET    0X8F
#define FT5216_REG_DOUBLE_CLICK_TIME_MIN  0X90
#define FT5216_REG_SINGLE_CLICK_TIME      0X91
#define FT5216_REG_LEFT_RIGHT_OFFSET      0X92
#define FT5216_REG_UP_DOWN_OFFSET         0X93
#define FT5216_REG_DISTANCE_LEFT_RIGHT    0X94
#define FT5216_REG_DISTANCE_UP_DOWN       0X95
#define FT5216_REG_ZOOM_DIS_SQR           0X96
#define FT5216_REG_RADIAN_VALUE           0X97
#define FT5216_REG_MAX_X_HIGH             0X98
#define FT5216_REG_MAX_X_LOW              0X99
#define FT5216_REG_MAX_Y_HIGH             0X9A
#define FT5216_REG_MAX_Y_LOW              0X9B
#define FT5216_REG_K_X_HIGH               0X9C
#define FT5216_REG_K_X_LOW                0X9D
#define FT5216_REG_K_Y_HIGH               0X9E
#define FT5216_REG_K_Y_LOW                0X9F
#define FT5216_REG_AUTO_CLB_MODE          0XA0
#define FT5216_REG_LIB_VERSION_H          0XA1
#define FT5216_REG_LIB_VERSION_L          0XA2
#define FT5216_REG_CIPHER                 0XA3
#define FT5216_REG_MODE                   0XA4
#define FT5216_REG_PMODE                  0XA5   /* Power Consume Mode        */
#define FT5216_REG_FIRMID                 0XA6   /* Firmware version */
#define FT5216_REG_STATE                  0XA7
#define FT5216_REG_FT5201ID               0XA8
#define FT5216_REG_ERR                    0XA9
#define FT5216_REG_CLB                    0XAA


#endif

