#ifndef _PID_H_
#define _PID_H_

#include "stm32f1xx_hal.h"

#define ratedSpd 1700     //常用速度，r/min

extern float Motor_PID(float set, float now, uint8_t clear);

#endif