#include "Laser.h"
#include <stdlib.h>
#include <string.h>

UART_HandleTypeDef *husart_laser1 = &huart4;  // 串口4
UART_HandleTypeDef *husart_laser2 = &husart2; // 串口2

float distance_laser1;
float distance_laser2;

uint8_t buf_laser1[10];
uint8_t buf_laser2[10];

uint8_t flag1_laser1 = 0;
uint8_t flag2_laser1 = 0;
uint8_t flag1_laser2 = 0;
uint8_t flag2_laser2 = 0;

uint8_t count_laser1 = 0;
uint8_t count_laser2 = 0;

void LASER_Init()
{
    //设置激光1和激光2的量程
    laser_stop_measure(LASER1);
    HAL_Delay(100);
    laser_set_range(LASER1, 9000);
    HAL_Delay(100);
    laser_start_measure(LASER1, 2); //设置频率
    HAL_Delay(100);

    // laser_stop_measure(LASER2);
    // HAL_Delay(100);
    // laser_set_range(LASER2, 9000);
    // HAL_Delay(100);
    // laser_start_measure(LASER2, 2); //设置频率
    // HAL_Delay(100);
}

float getDistance(uint8_t LASER)
{
    if (LASER == LASER1)
    {
        return distance_laser1;
    }
    else if (LASER == LASER2)
    {
        return distance_laser2;
    }
    else
    {
        return -1;
    }
}

// 开始测量
// frequence 为0、1或2
// 0： 10hz
// 1： 20hz
// 2： 30hz
void laser_start_measure(uint8_t LASER, uint8_t frequence)
{
    uint8_t cmd[10] = "iFACM:";
    cmd[6] = '0' + frequence;
    if (LASER == LASER1)
    {
        HAL_UART_Transmit(husart_laser1, cmd, 7, 1000);
    }
    else if (LASER == LASER2)
    {
        HAL_UART_Transmit(husart_laser2, cmd, 7, 1000);
    }
}

// 停止测量
void laser_stop_measure(uint8_t LASER)
{
    if (LASER == LASER1)
    {
        while (laser_send_cmd(husart_laser1, "iHALT", "OK", 1000))
            ;
    }
    else if (LASER == LASER2)
    {
        while (laser_send_cmd(husart_laser2, "iHALT", "OK", 1000))
            ;
    }
}

// 设置量程
// range 最大为80000 单位为 mm
void laser_set_range(uint8_t LASER, uint32_t range)
{
    char rangeStr[13] = {0};
    char cmd[20] = "iSET:2,";
    sprintf(rangeStr, "%d", range);
    memcpy(cmd + 7, rangeStr, strlen(rangeStr));
    if (LASER == LASER1)
    {
        while (laser_send_cmd(husart_laser1, (uint8_t *)cmd, "OK", 2000))
            ;
    }
    else if (LASER == LASER2)
    {
        while (laser_send_cmd(husart_laser2, (uint8_t *)cmd, "OK", 2000))
            ;
    }
}

//向ATK-ESP8266发送命令
//cmd:发送的命令字符串
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
//向串口三发送cmd（添加回车），若串口三接收到回复就用串口一发送出来
uint8_t laser_send_cmd(UART_HandleTypeDef *husart_laser, uint8_t *cmd, uint8_t *ack, uint16_t waittime)
{
    uint8_t res = 1;
    char *strx = NULL;
    uint8_t receive_count = 0;
    static uint8_t rx_buf[MAX_RECV_LEN];

    HAL_UART_Transmit(husart_laser, cmd, strlen((char *)cmd), 2000);

    do
    {
        HAL_UART_Receive(husart_laser, &rx_buf[receive_count], 1, 10);
        if (rx_buf[receive_count] != 0) //等待时间内收到字符则移动字符缓冲指针并把空闲等待时间重置，留一个结束符位置
        {
            if (receive_count < MAX_RECV_LEN - 1) //防止超出缓冲区
                receive_count++;
            if ((receive_count % strlen((const char *)ack)) == 0) //每当接收应答字符长度的整数倍个字符串时进行一次校验
            {
                strx = strstr((const char *)rx_buf, (const char *)ack); //匹配字符串
                if (strx)
                {
                    res = 0;
                    memset(rx_buf, 0, receive_count);
                    break;
                }
            }
        }
        waittime--;
    } while (waittime > 0);

    return res;
}
