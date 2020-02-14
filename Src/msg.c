#include "msg.h"
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 50
//cmd:发送的命令字符串
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
//向串口三发送cmd（添加回车），若串口三接收到回复就用串口一发送出来
uint8_t send_cmd(UART_HandleTypeDef *husart, uint8_t *cmd, uint8_t *ack, uint16_t waittime)
{
    uint8_t res = 1;
    char *strx = NULL;
    uint8_t receive_count = 0;
    static uint8_t rx_buf[MAX_LEN];

    HAL_UART_Transmit(husart, cmd, strlen((char *)cmd), 10);

    do
    {
        HAL_UART_Receive(husart, &rx_buf[receive_count], 1, 10);
        if (rx_buf[receive_count] != 0) //等待时间内收到字符则移动字符缓冲指针并把空闲等待时间重置，留一个结束符位置
        {
            if (receive_count < (MAX_LEN - 1)) //防止超出缓冲区
                receive_count++;
            else
            {
                memset(rx_buf, 0, receive_count);
                receive_count = 0;
            }
            // if ((receive_count % strlen((const char *)ack)) == 0) //每当接收应答字符长度的整数倍个字符串时进行一次校验
            // {
            strx = strstr((const char *)rx_buf, (const char *)ack); //匹配字符串
            if (strx)
            {
                res = 0;
                memset(rx_buf, 0, MAX_LEN);
                break;
            }
            // }
        }
        waittime--;
    } while (waittime > 0);

    return res;
}

//接收完成回复并重复应答，接收到OK后返回0
uint8_t rec_repeat(UART_HandleTypeDef *husart, uint8_t *rec)
{
    static uint8_t RX_Buf[MAX_LEN] = {0};
    uint8_t RX_Buf_Pt = 0;
    char *strx = 0;

    do
    {
        HAL_UART_Receive(husart, &RX_Buf[RX_Buf_Pt], 1, 10);
        if (RX_Buf[RX_Buf_Pt] != 0)
        {
            RX_Buf_Pt++;
            strx = strstr((const char *)RX_Buf, (const char *)rec);
            if (strx)
            {
                memset(RX_Buf, 0, MAX_LEN);
                break;
            }
            if (RX_Buf_Pt >= MAX_LEN)
            {
                RX_Buf_Pt = 0;
                memset(RX_Buf, 0, MAX_LEN);
            }
        }
    } while (1);

    HAL_UART_Transmit(husart, rec, strlen((char *)rec), 10);
    HAL_Delay(200);
    HAL_UART_Transmit(husart, rec, strlen((char *)rec), 10);
    HAL_Delay(200);
    HAL_UART_Transmit(husart, rec, strlen((char *)rec), 10);
    HAL_Delay(200);
    return 0;
}