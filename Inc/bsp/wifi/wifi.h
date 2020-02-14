#ifndef _WIFI_H
#define _WIFI_H

#include "headfile.h"

#define wifi_MAX_RECV_LEN 400                  //最大接收缓存字节数
#define maxcJsonMsg 100                        //最大cJson消息接收缓存字节数
extern uint8_t wifi_RX_BUF[wifi_MAX_RECV_LEN]; //接收缓存区
extern uint16_t wifi_RX_STA;
extern uint8_t RX_EN; //接收使能

struct Location
{
    char side;
    float distance;
    float height;
    float width;
};

extern int GotGoodsResult;//取货结果 初始化为0 0表示成功 1表示失败
extern int LoseGoodsResult;//卸货结果 初始化为0 0表示成功 1表示失败

extern struct Location goodsLocation;

extern uint8_t atk_8266_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t idletime);
extern uint8_t atk_8266_quit_trans(void);
extern uint8_t wifi_init(void);
extern void wifi_send(uint8_t *str, uint16_t len, uint32_t waittime);
extern void wifi_rec(uint8_t *str, uint16_t len, uint32_t waittime);
extern void send_ID(void);
extern void wifiMsg_rec(void);
extern void wifiMsg_Parse(uint8_t goalType);
extern void feedBackTypeGot(int buinessTypeGot);
extern void checkSysState(void);
extern void feedbackStartGetGoods(void);
extern void feedbackGotGoodsResult(void);
extern void feedbackLoseGoodsResult(void);
extern void feedbackGoInit(void);
#endif