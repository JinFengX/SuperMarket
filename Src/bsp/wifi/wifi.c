#include "headfile.h"

// uint8_t wifista_ssid_password[] = "AT+CWJAP=\"AIWAC0.9\",\"epic2019\"";      //路由器SSI,连接密码
uint8_t wifista_ssid_password[] = "AT+CWJAP=\"BigLun\",\"21612161\"";      //路由器SSI,连接密码
// uint8_t wifista_ssid_password[] = "AT+CWJAP=\"AiwacMarket\",\"aiwac2019\""; //路由器SSI,连接密码
// uint8_t wifi_IP_Portnum[] = "AT+CIPSTART=\"TCP\",\"172.16.0.105\",8899"; //服务端IP,服务端端口
uint8_t wifi_IP_Portnum[] = "AT+CIPSTART=\"TCP\",\"192.168.43.127\",8899"; //服务端IP,服务端端口

uint8_t wifi_RX_BUF[wifi_MAX_RECV_LEN] = {0}; //接收缓存区
uint16_t wifi_RX_STA = 0;
char cJsonMsg[maxcJsonMsg] = {0}; //cJson缓存区

struct Location goodsLocation;

//向ATK-ESP8266发送命令
//cmd:发送的命令字符串
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
//向串口三发送cmd（添加回车），若串口三接收到回复就用串口一发送出来
uint8_t atk_8266_send_cmd(uint8_t *cmd, uint8_t *ack, uint16_t waittime)
{
    uint8_t res = 1;
    char *strx = NULL;
    HAL_UART_Transmit(&wifiUart, cmd, strlen((char *)cmd), 2000);
    HAL_UART_Transmit(&wifiUart, "\r\n", 2, 1000);

    do
    {
        HAL_UART_Receive(&wifiUart, &wifi_RX_BUF[wifi_RX_STA], 1, 10);
        if (wifi_RX_BUF[wifi_RX_STA] != 0) //等待时间内收到字符则移动字符缓冲指针并把空闲等待时间重置
        {
            wifi_RX_STA++;
            strx = strstr((const char *)wifi_RX_BUF, (const char *)ack); //匹配字符串
            if (strx)
            {
                res = 0;
                memset(wifi_RX_BUF, 0, wifi_MAX_RECV_LEN);
                wifi_RX_STA = 0;
                break;
            }
            if (wifi_RX_STA == wifi_MAX_RECV_LEN - 1)                         //防止超出缓冲区
            {
                wifi_RX_STA = 0;
                memset(wifi_RX_BUF, 0, wifi_MAX_RECV_LEN);///
            }
        }
        waittime--;
    } while (waittime > 0);
    
    return res;
}

//ATK-ESP8266退出透传模式
//返回值:0,退出成功;
//       1,退出失败
uint8_t atk_8266_quit_trans(void)
{
    HAL_UART_Transmit(&wifiUart, "+++", 3, 1000); //退出透传
    HAL_Delay(100);
    return atk_8266_send_cmd("AT", "OK", 50); //退出透传判断.
}

uint8_t wifi_init(void)
{
    while (atk_8266_send_cmd("AT", "OK", 50))
    {
        atk_8266_quit_trans();                       //连接失败则退出透传模式
        atk_8266_send_cmd("AT+CIPMODE=0", "OK", 50); //关闭透传模式
        HAL_Delay(100);
    }
    while (atk_8266_send_cmd("ATE0", "OK", 50)) ;//关闭回显
    HAL_Delay(10);
    while (atk_8266_send_cmd("AT+CWMODE=1", "OK", 50))
        ;                                  //设置WIFI STA模式
    atk_8266_send_cmd("AT+RST", "OK", 50); //DHCP服务器关闭(仅AP模式有效)
    HAL_Delay(3000);                       //延时3S等待重启成功

    while (atk_8266_send_cmd(wifista_ssid_password, "WIFI GOT IP", 500)) //////
        ;
    while (atk_8266_send_cmd("AT+CIPMUX=0", "OK", 50)) //0：单连接，1：多连接
        ;
    while (atk_8266_send_cmd(wifi_IP_Portnum, "OK", 200)) //连接服务端
        ;
    while (atk_8266_send_cmd("AT+CIPMODE=1", "OK", 50)) //传输模式为：透传
        ;
    while (atk_8266_send_cmd("AT+CIPSEND", ">", 100)) //开始透传
        ;

    return 1;
}

//获取ATK-ESP8266模块的连接状态
//返回值:0,未连接;1,连接成功.
uint8_t atk_8266_consta_check(void)
{
    uint8_t res;
    if (atk_8266_quit_trans())
        return 0;                                          //退出透传
    if (atk_8266_send_cmd("AT+CIPSTATUS", "OK", 100) == 0) //发送AT+CIPSTATUS指令,查询连接状态
        res = 1;                                           //得到连接状态
    return res;
}

void wifi_send(uint8_t *str, uint16_t len, uint32_t waittime)
{
    HAL_UART_Transmit(&wifiUart, str, len, waittime);
}

void wifi_rec(uint8_t *str, uint16_t len, uint32_t waittime)
{
    HAL_UART_Receive(&wifiUart, str, len, waittime);
}

//向服务端发送取货机器人单元号
void send_ID(void)
{
    cJSON *root, *data;
    int num = 0;

    char *strSend;
    char send[200];

    root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "businessType", "0000");
    cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());
    cJSON_AddStringToObject(data, "ID", "Area1");
    strSend = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    // 去掉所有\r\n.安卓端是  一行一行的接收
    num = strlen(strSend);
    for (int numS = 0; numS < num; numS++)
    {
        if ((strSend[numS] == '\n') || (strSend[numS] == '\r'))
        {
            strSend[numS] = ' ';
        }
    }

    strSend[num] = '\n';

    // 加协议头
    memset(send, 0, sizeof(send));
    send[0] = '#';
    send[1] = '!';
    strncpy(send + 2, strSend, num);
    send[num + 2] = '&';
    send[num + 3] = '\n';

    wifi_send((uint8_t *)send, strlen((const char *)send), 1000);

    myfree(strSend);
}

//从服务端消息接收并提取cJson字符串
void wifiMsg_rec(void)
{
    while (1)
    {
        wifi_rec(&wifi_RX_BUF[wifi_RX_STA], 1, 100); //等待接收一个字符
        if (wifi_RX_BUF[wifi_RX_STA] == '#')         //第一个字节校验
        {
            //第一个字节校验正确
            wifi_rec(&wifi_RX_BUF[++wifi_RX_STA], 1, 100); //等待接收第二个字符
            if (wifi_RX_BUF[wifi_RX_STA] == '!')           //校验字头
            {
                //表头校验成功，继续接收
                while ((wifi_RX_BUF[wifi_RX_STA - 1] != '&') && (wifi_RX_BUF[wifi_RX_STA] != '\r') && (wifi_RX_STA <= (maxcJsonMsg - 1))) //接收直到收到'\r' 或缓存满
                {
                    wifi_rec(&wifi_RX_BUF[++wifi_RX_STA], 1, 10);
                }

                if ((wifi_RX_BUF[wifi_RX_STA - 1] == '&') || (wifi_RX_BUF[wifi_RX_STA] == '\r')) //表尾接收校验
                {
                    //表尾校验正确
                    strncpy(cJsonMsg, (const char *)&wifi_RX_BUF[2], wifi_RX_STA - 3); //缓存提取
                    memset(wifi_RX_BUF, 0, wifi_RX_STA);                               //重置缓存
                    wifi_RX_STA = 0;
                    break; //唯一出口
                }
                memset(wifi_RX_BUF, 0, wifi_RX_STA); //重置缓存
                wifi_RX_STA = 0;
            }
            else
            {
                //第二个字节校验错误则重置前两个字节并返回第一个字符接收
                wifi_RX_BUF[0] = wifi_RX_BUF[1] = wifi_RX_STA = 0;
            }
        }
        //第一个字节校验错误 返回第一个字符接收
    }
}

//对接收到的cJson数据进行解析
//para 解析接口类型
int8_t businessType = -1;
void wifiMsg_Parse(uint8_t goalType)
{
    cJSON *root, *orderValue, *data;

    while (1)
    {
        wifiMsg_rec(); //提取cJson字符串

        root = cJSON_Parse(cJsonMsg);
        if (!root)
        {
            //解析失败重新接收消息
            continue;
        }

        orderValue = cJSON_GetObjectItem(root, "businessType");
        if (!orderValue)
        {
            //解析失败重新接收消息
            cJSON_Delete(root);
            continue;
        }

        businessType = atoi(orderValue->valuestring);

        if (businessType == goalType) //所得接口类型和目标类型一致性验证
        {
            //接口验证一致
            if (goalType == 1)
            {
                //1类消息解析成功 删除根返回
                cJSON_Delete(root);
                return;
            }
            if (goalType == 3)
            {
                data = cJSON_GetObjectItem(root, "data");
                if (!data)
                {
                    //解析失败重新接收消息
                    cJSON_Delete(root);
                    continue;
                }
                orderValue = cJSON_GetObjectItem(data, "side");
                if (!orderValue)
                {
                    //解析失败重新接收消息
                    cJSON_Delete(root);
                    continue;
                }
                strcpy(&goodsLocation.side, orderValue->valuestring);

                orderValue = cJSON_GetObjectItem(data, "distance");
                if (!orderValue)
                {
                    //解析失败重新接收消息
                    cJSON_Delete(root);
                    continue;
                }
                goodsLocation.distance = atof(orderValue->valuestring) / 1000;

                orderValue = cJSON_GetObjectItem(data, "height");
                if (!orderValue)
                {
                    //解析失败重新接收消息
                    cJSON_Delete(root);
                    continue;
                }
                goodsLocation.height = atof(orderValue->valuestring) / 1000;

                orderValue = cJSON_GetObjectItem(data, "depth");
                if (!orderValue)
                {
                    //解析失败重新接收消息
                    cJSON_Delete(root);
                    continue;
                }
                goodsLocation.width = atof(orderValue->valuestring) / 1000;

                //解析完成 删除根返回
                cJSON_Delete(root);
                return;
            }
        }
        else
        {
            //接口验证不一致 重新接收
            cJSON_Delete(root);
            continue;
        }

        HAL_Delay(100);
    }
}

//反馈给服务端已收到的接口类型
//para 反馈接口类型
void feedBackTypeGot(int buinessTypeGot)
{
    cJSON *root, *data;
    char TypeGot[6] = {0};
    int num = 0;

    char *strSend;
    char send[100];

    sprintf(TypeGot, "%04d", buinessTypeGot);

    //	给服务器发状�?
    root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "businessType", "0024");
    cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());

    cJSON_AddStringToObject(data, "businessTypeGot", TypeGot);

    strSend = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // 去掉所有\r\n.安卓端是  一行一行的接收
    num = strlen(strSend);
    for (int numS = 0; numS < num; numS++)
    {
        if ((strSend[numS] == '\n') || (strSend[numS] == '\r'))
        {

            strSend[numS] = ' ';
        }
    }

    strSend[num] = '\n';

    // 加协议头
    memset(send, 0, sizeof(send));
    send[0] = '#';
    send[1] = '!';
    strncpy(send + 2, strSend, num);
    send[num + 2] = '&';
    send[num + 3] = '\n';

    wifi_send((uint8_t *)send, strlen(send), 100);

    myfree(strSend);
    HAL_Delay(150);
}

//函数功能:检查系统的 状态，主控需要检查当前的位置是否在复位点
void checkSysState(void)
{
    int ret = 1; // 状态标志	1:ok  0:error
    char errorDesc[500];

    cJSON *root, *data;

    int num = 0;
    int numS = 0;

    char *strSend;
    char send[200];

    //留给错误检测

    //  给服务器发状态
    root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "businessType", "0001");
    cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());

    if (ret == 0)
    {
        cJSON_AddStringToObject(data, "status", "0");
        cJSON_AddStringToObject(data, "errorDesc", errorDesc);
    }
    else
    {
        cJSON_AddStringToObject(data, "status", "4");
        cJSON_AddStringToObject(data, "errorDesc", "ok");
    }

    strSend = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // 去掉所有\r\n.安卓端是  一行一行的接收
    num = strlen(strSend);
    for (numS = 0; numS < num; numS++)
    {
        if ((strSend[numS] == '\n') || (strSend[numS] == '\r'))
        {

            strSend[numS] = ' ';
        }
    }
    strSend[num] = '\n';

    // 加协议头
    memset(send, 0, sizeof(send));
    send[0] = '#';
    send[1] = '!';
    strncpy(send + 2, strSend, num);
    send[num + 2] = '&';
    send[num + 3] = '\n';

    wifi_send((uint8_t *)send, strlen(send), 100);
    myfree(strSend);

    //若状态有问题，停止在这个函数
    while (1)
    {
        if (ret)
            break;
        HAL_Delay(1000);
    }
}

//******************************向服务端反馈开始取货
void feedbackStartGetGoods(void)
{

    cJSON *root, *data;

    int num = 0;
    int numS = 0;

    char *strSend;
    char send[200];

    //	给服务器发状态
    root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "businessType", "0004");
    cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());
    cJSON_AddStringToObject(data, "status", "1");
    cJSON_AddStringToObject(data, "errorDesc", "fff");

    strSend = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // 去掉所有\r\n.安卓端是  一行一行的接收
    num = strlen(strSend);
    for (numS = 0; numS < num; numS++)
    {
        if ((strSend[numS] == '\n') || (strSend[numS] == '\r'))
        {

            strSend[numS] = ' ';
        }
    }

    strSend[num] = '\n';

    // 加协议头
    memset(send, 0, sizeof(send));
    send[0] = '#';
    send[1] = '!';
    strncpy(send + 2, strSend, num);
    send[num + 2] = '&';
    send[num + 3] = '\n';

    wifi_send((uint8_t *)send, strlen(send), 100);
    myfree(strSend);
}

//**********************************向服务端反馈取货结果
int GotGoodsResult = 0;
void feedbackGotGoodsResult(void)
{

    cJSON *root, *data; //

    int num = 0;
    int numS = 0;

    char *strSend;
    char send[200];

    root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "businessType", "0005");
    cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());

    if (GotGoodsResult == 0)
    {
        cJSON_AddStringToObject(data, "status", "2");
    }
    else
    {
        cJSON_AddStringToObject(data, "status", "0");
    }

    cJSON_AddStringToObject(data, "errorDesc", "fff");

    strSend = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // 去掉所有\r\n.安卓端是  一行一行的接收
    num = strlen(strSend);
    for (numS = 0; numS < num; numS++)
    {
        if ((strSend[numS] == '\n') || (strSend[numS] == '\r'))
        {

            strSend[numS] = ' ';
        }
    }

    strSend[num] = '\n';

    // 加协议头
    memset(send, 0, sizeof(send));
    send[0] = '#';
    send[1] = '!';
    strncpy(send + 2, strSend, num);
    send[num + 2] = '&';
    send[num + 3] = '\n';

    wifi_send((uint8_t *)send, strlen(send), 100);
    myfree(strSend);
}

//*************************向服务端反馈卸货结果
int LoseGoodsResult = 0;
void feedbackLoseGoodsResult(void)
{

    cJSON *root, *data;

    int num = 0;
    int numS = 0;

    char *strSend;
    char send[200];

    root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "businessType", "0006");
    cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());

    if (LoseGoodsResult == 0)
    {
        cJSON_AddStringToObject(data, "status", "3");
    }
    else
    {
        cJSON_AddStringToObject(data, "status", "0");
    }

    cJSON_AddStringToObject(data, "errorDesc", "fff");

    strSend = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // 去掉所有\r\n.安卓端是  一行一行的接收
    num = strlen(strSend);
    for (numS = 0; numS < num; numS++)
    {
        if ((strSend[numS] == '\n') || (strSend[numS] == '\r'))
        {

            strSend[numS] = ' ';
        }
    }

    strSend[num] = '\n';

    // 加协议头
    memset(send, 0, sizeof(send));
    send[0] = '#';
    send[1] = '!';
    strncpy(send + 2, strSend, num);
    send[num + 2] = '&';
    send[num + 3] = '\n';

    wifi_send((uint8_t *)send, strlen(send), 100);
    myfree(strSend);
}

//***********************反馈完成取货并复位
void feedbackGoInit(void)
{

    cJSON *root, *data; //

    int num = 0;
    int numS = 0;

    char *strSend;
    char send[200];

    root = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "businessType", "0002");
    cJSON_AddItemToObject(root, "data", data = cJSON_CreateObject());
    cJSON_AddStringToObject(data, "status", "4");
    cJSON_AddStringToObject(data, "errorDesc", "fff");

    strSend = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    // 去掉所有\r\n.安卓端是  一行一行的接收
    num = strlen(strSend);
    for (numS = 0; numS < num; numS++)
    {
        if ((strSend[numS] == '\n') || (strSend[numS] == '\r'))
        {

            strSend[numS] = ' ';
        }
    }

    strSend[num] = '\n';

    // 加协议头
    memset(send, 0, sizeof(send));
    send[0] = '#';
    send[1] = '!';
    strncpy(send + 2, strSend, num);
    send[num + 2] = '&';
    send[num + 3] = '\n';

    wifi_send((uint8_t *)send, strlen(send), 100);
    myfree(strSend);
}
