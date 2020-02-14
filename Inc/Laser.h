#ifndef _Laser_H
#define _Laser_H

#include "usart/bsp_usartx.h"

#define LASER1 1
#define LASER2 2
#define MAX_RECV_LEN 100

extern UART_HandleTypeDef *husart_laser1 ;  // 串口4
extern UART_HandleTypeDef *husart_laser2 ; // 串口1

extern float distance_laser1;
extern float distance_laser2;

extern uint8_t buf_laser1[10];
extern uint8_t buf_laser2[10];

extern uint8_t flag1_laser1;
extern uint8_t flag2_laser1;
extern uint8_t flag1_laser2;
extern uint8_t flag2_laser2;

extern uint8_t count_laser1;
extern uint8_t count_laser2;

void LASER_Init();

float getDistance(uint8_t LASER);
void laser_start_measure(uint8_t, uint8_t);
void laser_stop_measure(uint8_t);
void laser_set_range(uint8_t LASER, uint32_t range);
uint8_t laser_send_cmd(UART_HandleTypeDef *husart_laser, uint8_t *cmd, uint8_t *ack, uint16_t waittime);
#endif