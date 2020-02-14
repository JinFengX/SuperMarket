#ifndef _Msg_H_
#define _Msg_H_

#include "stm32f1xx_hal.h"

extern uint8_t send_cmd(UART_HandleTypeDef *husart, uint8_t *cmd, uint8_t *ack, uint16_t waittime);
extern uint8_t rec_repeat(UART_HandleTypeDef *husart, uint8_t *rec);


#endif