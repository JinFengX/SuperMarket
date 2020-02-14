#ifndef __BSP_USARTX_H__
#define __BSP_USARTX_H__

/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* ���Ͷ��� ------------------------------------------------------------------*/
/* �궨�� --------------------------------------------------------------------*/
#define wifiUart husart2

/* ��չ���� ------------------------------------------------------------------*/
extern UART_HandleTypeDef husart1;
extern UART_HandleTypeDef husart2;
extern UART_HandleTypeDef husart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern DMA_HandleTypeDef hdma_rs485_rx;
/* �������� ------------------------------------------------------------------*/
void MX_USARTx_Init(void);

#endif /* __BSP_USARTX_H__ */

/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
