#ifndef __BSP_USARTX_H__
#define __BSP_USARTX_H__

/* 包含头文件 ----------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* 类型定义 ------------------------------------------------------------------*/
/* 宏定义 --------------------------------------------------------------------*/
#define wifiUart husart2

/* 扩展变量 ------------------------------------------------------------------*/
extern UART_HandleTypeDef husart1;
extern UART_HandleTypeDef husart2;
extern UART_HandleTypeDef husart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern DMA_HandleTypeDef hdma_rs485_rx;
/* 函数声明 ------------------------------------------------------------------*/
void MX_USARTx_Init(void);

#endif /* __BSP_USARTX_H__ */

/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
