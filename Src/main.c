#include "stm32f1xx_hal.h"
#include "usart/bsp_usartx.h"
#include "string.h"
#include "stdlib.h"
#include "ASDA_B2/bsp_ASDA_B2.h"
#include "Laser.h"
#include "PID.h"
#include "math.h"
#include "msg.h"
#include "stmflash/stm_flash.h"
#include "headfile.h"
/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
__IO uint8_t Rx_Buf[50]; //接收数据缓存
uint8_t aRxBuffer_laser1;
uint8_t aRxBuffer_laser2;

/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/
/**
  * 函数功能: 系统时钟配置
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE; // 外部晶振，8MHz
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9; // 9倍频，得到72MHz主时钟
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // 系统时钟：72MHz
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;        // AHB时钟：72MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;         // APB1时钟：36MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;         // APB2时钟：72MHz
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

    // HAL_RCC_GetHCLKFreq()/1000    1ms中断一次
    // HAL_RCC_GetHCLKFreq()/100000	 10us中断一次
    // HAL_RCC_GetHCLKFreq()/1000000 1us中断一次
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000); // 配置并启动系统滴答定时器
    /* 系统滴答定时器时钟源 */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    /* 系统滴答定时器中断优先级配置 */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
  * 函数功能: 主函数.
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明: 无
  */
uint8_t test;

//设定速度为正值时，链条正向旋转，因此将激光所指方向放置为顺时针，参照点为正方向的四个顶点
//测量距离为当前面到参照点的距离
enum Side //区域定义
{
    A, //复位点所在面
    B, //出货点所在面
    C  //取货点所在面
};
#define initDistance 0.6      //初始距离
#define outDistance 1.1       //出货距离
const enum Side initSide = A; //初始面
enum Side nowSide = A;        //初始化为复位点所在面
enum Side aimSide = A;        //初始化目标面为复位点所在面
uint8_t sideChange = 0;
int dir = 0;                  //前进方向
#define filter 0.2            //平滑系数，系数越大越平滑
float aimDistance = 0;        //目标激光距离
float distance_L1 = 0.0;      //当前激光距离
float lastDistance_L1 = 0.0;  //上次激光读取距离
float lastDistance2_L1 = 0.0; //上上次激光读取距离
float distance_L2 = 0.0;      //当前激光距离
float lastDistance_L2 = 0.0;  //上次激光读取距离
float lastDistance2_L2 = 0.0; //上上次激光读取距离
uint8_t distanceMin = 0;      //最短距离标志位
float setSpd = 0;             //设定速度
float height = 0;             //高度
float width = 0;              //宽度

const float AtoBLenth = 1.85;
const float BtoCLenth = 2.1;
const float CtoBLenth = 0.6;
const float BtoALenth = 0.5;

const float crtDistance = 0.06;  //矫正范围，进入矫正范围使用位置——速度串级控制，否则使用单速度闭环控制
const float allowedError = 0.02; //允许误差范围，单位米

uint8_t Process = 0x00; //流程标志

int dirSet(enum Side aimside, enum Side nowside);

int main(void)
{
    // HAL_Init();
    // /* 配置系统时钟 */
    // SystemClock_Config();

    // /* 初始化串口并配置串口中断优先级 */
    // MX_USARTx_Init();
    // my_mem_init(SRAMIN); //初始化内部内存池

    // wifi_init();
    // HAL_Delay(1000);

    //  send_ID();
    // wifiMsg_Parse(1);
    // feedBackTypeGot(1);
    // checkSysState();

    // wifiMsg_Parse(3);
    // feedBackTypeGot(3);
    // feedbackStartGetGoods();
    // GotGoodsResult = 0;
    // feedbackGotGoodsResult();
    // LoseGoodsResult = 0;
    // feedbackLoseGoodsResult();
    // feedbackGoInit();

    // while (1)
    // {
    // }
    /////////////////////////////////////////////

    /* 复位所有外设，初始化Flash接口和系统滴答定时器 */
    HAL_Init();
    /* 配置系统时钟 */
    SystemClock_Config();

    /* 初始化串口并配置串口中断优先级 */
    MX_USARTx_Init();

    // wifi初始化
    my_mem_init(SRAMIN); //初始化内部内存池
    wifi_init();
    HAL_Delay(1000);

    // 向安卓发送主控id
    send_ID();

    if (!flash_read(&Process, 1)) //读取返回值为0 流程正常
    {
        Process = 0;
    }

    if (Process != 0x00) //上次未处理完成订单或异常未处理 进入异常处理流程
    {
        //清除上次未完成标志或异常标志 写入异常处理标志
        flash_clear_used(1);
        Process = 0x01;
        flash_write_new(&Process, 1);

        //通知服务端异常

        while (Process) //异常处理
        {
            //等待服务端通知异常处理完成
            HAL_Delay(3000);
            HAL_Delay(3000);

            //异常清除
            flash_clear_used(1);
            if (!flash_read(&Process, 1)) //读取返回值为0时 说明异常已经确认清除
            {
                Process = 0;
            }//Process为0 跳出循环继续正常流程
        }
    }

    // 接收安卓的状态查询指令
    wifiMsg_Parse(1);
    feedBackTypeGot(1);

    while (rec_repeat(&huart5, "CRdyOK@")) //等待取货单元初始化完成
        ;

    // 激光初始化
    LASER_Init();
    HAL_UART_Receive_IT(husart_laser1, &aRxBuffer_laser1, 1); // 激光1的串口 开中断
    HAL_Delay(500);
    // HAL_UART_Receive_IT(husart_laser2, &aRxBuffer_laser2, 1); // 激光2的串口 开中断
    // HAL_Delay(500);

    //电机初始化
    HAL_UART_Receive_DMA(&husart3, (uint8_t *)&test, 1); // Data Direction: 485 --> USART1
    /* Disable the Half transfer complete interrupt */
    __HAL_DMA_DISABLE_IT(&hdma_rs485_rx, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(&hdma_rs485_rx, DMA_IT_TE);
    /* 初始化ASDA-B2参数,配置为速度位置模式*/
    ASDAB2_Init();
    /* 设置SP3速度值为600*0.1r/min  60r/min */
    SetSpeed(REG_SP3, 0);
    /* 启动伺服 */
    StartServo();
    HAL_Delay(2000); //两秒后启动

    // 告知安卓主控的状态
    checkSysState();

    // while (1)
    // {
    //   height = 1.0;
    //   width = 0.5;
    //   uint8_t Dat[20] = {0};
    //   //发送高度
    //   sprintf(Dat, "DH%.5f!", height);
    //   while (send_cmd(&huart5, Dat, Dat, 1000)) //等待回复
    //     ;
    //   HAL_UART_Transmit(&huart5, "OK", 2, 1000);
    //   memset(Dat, 0, 20);

    //   sprintf(Dat, "DW%.5f", width);
    //   while (send_cmd(&huart5, Dat, Dat, 1000))
    //     ;
    //   HAL_UART_Transmit(&huart5, "OK", 2, 1000);
    //   memset(Dat, 0, 20);

    //   while (send_cmd(&huart5, "CGet@", "CGet@", 1000))
    //     ;
    //   HAL_UART_Transmit(&huart5, "OK", 2, 1000);
    //   while (rec_repeat(&huart5, "CGetOK@"))
    //     ;

    //   while (send_cmd(&huart5, "CPut@", "CPut@", 1000))
    //     ;
    //   HAL_UART_Transmit(&huart5, "OK", 2, 1000);
    //   while (rec_repeat(&huart5, "CPutOK@"))
    //     ;

    //   while (send_cmd(&huart5, "CThw@", "CThw@", 1000))
    //     ;
    //   HAL_UART_Transmit(&huart5, "OK", 2, 1000);
    //   while (rec_repeat(&huart5, "CThwOK@"))
    //     ;
    // }

    // while(1);

    while (1)
    {
        // 接收安卓发来的货物位置信息
        wifiMsg_Parse(3);
        feedBackTypeGot(3);

        //*****************************************初始信息读取
        nowSide = A;
        aimSide = goodsLocation.side - 'A';
        aimDistance = goodsLocation.distance;
        // while(1);/////////////////////

        // 告知安卓开始取货
        feedbackStartGetGoods();
        //流程状态写入flash
        Process = 0xFE;
        flash_write_new(&Process, 1);

        //******************************************前往取货点初始速度设定
        while (1)
        {
            dir = dirSet(aimSide, nowSide); //逆时针-1 顺时针1
            break;
        }

        //*******************************************出入弯判断
        while (1)
        {
            if (aimSide == nowSide)
            {
                sideChange = 0;
                HAL_Delay(200);
                break; //进入下一步
            }

            if (sideChange == 1) //换面后还未到达目标面则3秒内不重复判断
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                sideChange = 0;
            }

            if ((dir < 0) && //逆时针
                ((nowSide == A) && (distance_laser1 > AtoBLenth) ||
                 (nowSide == B) && (distance_laser1 > BtoCLenth))) //2测得距离小于最小边长则判定为即将逆向入弯
            {

                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                nowSide++;
                sideChange = 1;
            }

            if ((dir > 0) && //顺时针
                ((nowSide == C) && (distance_laser1 < CtoBLenth) ||
                 (nowSide == B) && (distance_laser1 < BtoALenth))) //1测得距离小于最小边长则判定为即将顺向入弯
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                nowSide--;
                sideChange = 1;
            }
        }

        //******************************************取货面矫正控制
        while (1)
        {
            distance_L1 = distance_laser1 * (1 - filter) + lastDistance_L1 * filter;
            if ((distance_L1 - aimDistance) > crtDistance) //控制策略选择
                setSpd = ratedSpd;
            else if ((distance_L1 - aimDistance) < (-1 * crtDistance)) //控制策略选择
                setSpd = -1 * ratedSpd;
            else if (fabs(distance_L1 - aimDistance) <= crtDistance)
                setSpd = Motor_PID(aimDistance, distance_L1, 0);
            SetSpeed(REG_SP3, setSpd);

            if ((fabs(distance_laser1 - aimDistance) < allowedError)) //&&
                                                                      // (fabs(lastDistance_L1 - aimDistance) < allowedError) &&
                                                                      // (fabs(lastDistance2_L1 - aimDistance) < allowedError)) //连续三帧满足误差范围要求进行下一步
            {
                SetSpeed(REG_SP3, 0); //停止电机
                setSpd = 0;
                Motor_PID(0, 0, 1); //清除历史值
                break;
            }
            HAL_Delay(20);
        }
        HAL_Delay(666);

        //***********************发送货物高度深度以及取货命令
        height = goodsLocation.height;
        width = goodsLocation.width;
        uint8_t Dat[20] = {0};
        //发送高度
        sprintf(Dat, "DH%.5f!", height);
        while (send_cmd(&huart5, Dat, Dat, 100)) //等待回复
            ;
        memset(Dat, 0, 20);
        HAL_Delay(10);

        sprintf(Dat, "DW%.5f", width);
        while (send_cmd(&huart5, Dat, Dat, 100))
            ;
        memset(Dat, 0, 20);
        HAL_Delay(10);

        while (send_cmd(&huart5, "CGet@", "CGet@", 100))
            ;
        HAL_Delay(10);

        while (rec_repeat(&huart5, "CGetOK@")) //等待取货完成
            ;

        // 告知安卓取货结果
        GotGoodsResult = 0;
        feedbackGotGoodsResult();

        //***************************************取货结束后前往出货面
        aimSide = B;
        aimDistance = 1.8;
        //***************************************前往出货点初始速度设定
        while (1)
        {
            dir = dirSet(aimSide, nowSide);
            break;
        }

        //*******************************************出入弯判断
        while (1)
        {
            if (aimSide == nowSide)
            {
                sideChange = 0;
                HAL_Delay(200);
                break; //进入下一步
            }

            if (sideChange == 1) //换面后还未到达目标面则3秒内不重复判断
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                sideChange = 0;
            }

            if ((dir < 0) && //逆时针
                ((nowSide == A) && (distance_laser1 > AtoBLenth) ||
                 (nowSide == B) && (distance_laser1 > BtoCLenth))) //2测得距离小于最小边长则判定为即将逆向入弯
            {

                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(500);
                nowSide++;
                sideChange = 1;
            }

            if ((dir > 0) && //顺时针
                ((nowSide == C) && (distance_laser1 < CtoBLenth) ||
                 (nowSide == B) && (distance_laser1 < BtoALenth))) //1测得距离小于最小边长则判定为即将顺向入弯
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(500);
                nowSide--;
                sideChange = 1;
            }
        }

        //***********************************出货面位置矫正
        while (1)
        {
            distance_L1 = distance_laser1 * (1 - filter) + lastDistance_L1 * filter;
            if ((distance_L1 - aimDistance) > crtDistance) //控制策略选择
                setSpd = ratedSpd;
            else if ((distance_L1 - aimDistance) < (-1 * crtDistance)) //控制策略选择
                setSpd = -ratedSpd;
            else if (fabs(distance_L1 - aimDistance) <= crtDistance)
                setSpd = Motor_PID(aimDistance, distance_L1, 0);
            SetSpeed(REG_SP3, setSpd);

            if ((fabs(distance_laser1 - aimDistance) < allowedError)) //&&
                                                                      // (fabs(lastDistance_L1 - aimDistance) < allowedError) &&
                                                                      // (fabs(lastDistance2_L1 - aimDistance) < allowedError)) //连续三帧满足误差范围要求进行下一步
            {
                SetSpeed(REG_SP3, 0); //停止电机
                setSpd = 0;
                Motor_PID(0, 0, 1); //清除历史值
                break;
            }
            HAL_Delay(20);
        }
        HAL_Delay(666);

        //*******************************发送卸货命令
        while (send_cmd(&huart5, "CPut@", "CPut@", 100))
            ;

        while (rec_repeat(&huart5, "CPutOK@")) //等待卸货完成
            ;

        // 告知安卓出货结果
        LoseGoodsResult = 0;
        feedbackLoseGoodsResult();

        //***************************************出货结束后前往卸盘点
        aimSide = B;
        aimDistance = 0.8;
        //***************************************前往卸盘点初始速度设定
        while (1)
        {
            dir = dirSet(aimSide, nowSide);
            break;
        }

        //*******************************************出入弯判断
        while (1)
        {
            if (aimSide == nowSide)
            {
                sideChange = 0;
                HAL_Delay(200);
                break; //进入下一步
            }

            if (sideChange == 1) //换面后还未到达目标面则3秒内不重复判断
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                sideChange = 0;
            }

            if ((dir < 0) && //逆时针
                ((nowSide == A) && (distance_laser1 > AtoBLenth) ||
                 (nowSide == B) && (distance_laser1 > BtoCLenth))) //2测得距离小于最小边长则判定为即将逆向入弯
            {

                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                nowSide++;
                sideChange = 1;
            }

            if ((dir > 0) && //顺时针
                ((nowSide == C) && (distance_laser1 < CtoBLenth) ||
                 (nowSide == B) && (distance_laser1 < BtoALenth))) //1测得距离小于最小边长则判定为即将顺向入弯
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                nowSide--;
                sideChange = 1;
            }
        }

        //***********************************位置矫正
        while (1)
        {
            distance_L1 = distance_laser1 * (1 - filter) + lastDistance_L1 * filter;
            if ((distance_L1 - aimDistance) > crtDistance) //控制策略选择
                setSpd = ratedSpd;
            else if ((distance_L1 - aimDistance) < (-1 * crtDistance)) //控制策略选择
                setSpd = -ratedSpd;
            else if (fabs(distance_L1 - aimDistance) <= crtDistance)
                setSpd = Motor_PID(aimDistance, distance_L1, 0);
            SetSpeed(REG_SP3, setSpd);

            if ((fabs(distance_laser1 - aimDistance) < allowedError)) //&&
                                                                      // (fabs(lastDistance_L1 - aimDistance) < allowedError) &&
                                                                      // (fabs(lastDistance2_L1 - aimDistance) < allowedError)) //连续三帧满足误差范围要求进行下一步
            {
                SetSpeed(REG_SP3, 0); //停止电机
                setSpd = 0;
                Motor_PID(0, 0, 1); //清除历史值
                break;
            }
            HAL_Delay(20);
        }

        //******************发送卸托盘命令
        while (send_cmd(&huart5, "CThw@", "CThw@", 100))
            ;

        while (rec_repeat(&huart5, "CThwOK@")) //等待卸托盘完成
            ;

        //********************************卸托盘完成回到初始点
        aimSide = A;
        aimDistance = 0.8;

        //***************************************前往复位点初始速度设定
        while (1)
        {
            dir = dirSet(aimSide, nowSide);
            break;
        }

        //*******************************************出入弯判断
        while (1)
        {
            if (aimSide == nowSide)
            {
                sideChange = 0;
                HAL_Delay(200);
                break; //进入下一步
            }

            if (sideChange == 1) //换面后还未到达目标面则3秒内不重复判断
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                sideChange = 0;
            }

            if ((dir < 0) && //逆时针
                ((nowSide == A) && (distance_laser1 > AtoBLenth) ||
                 (nowSide == B) && (distance_laser1 > BtoCLenth))) //2测得距离小于最小边长则判定为即将逆向入弯
            {

                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                nowSide++;
                sideChange = 1;
            }

            if ((dir > 0) && //顺时针
                ((nowSide == C) && (distance_laser1 < CtoBLenth) ||
                 (nowSide == B) && (distance_laser1 < BtoALenth))) //1测得距离小于最小边长则判定为即将顺向入弯
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                nowSide--;
                sideChange = 1;
            }
        }

        //***********************************位置矫正
        while (1)
        {
            distance_L1 = distance_laser1 * (1 - filter) + lastDistance_L1 * filter;
            if ((distance_L1 - aimDistance) > crtDistance) //控制策略选择
                setSpd = ratedSpd;
            else if ((distance_L1 - aimDistance) < (-1 * crtDistance)) //控制策略选择
                setSpd = -ratedSpd;
            else if (fabs(distance_L1 - aimDistance) <= crtDistance)
                setSpd = Motor_PID(aimDistance, distance_L1, 0);
            SetSpeed(REG_SP3, setSpd);

            if ((fabs(distance_laser1 - aimDistance) < allowedError)) //&&
                                                                      // (fabs(lastDistance_L1 - aimDistance) < allowedError) &&
                                                                      // (fabs(lastDistance2_L1 - aimDistance) < allowedError)) //连续三帧满足误差范围要求进行下一步
            {
                SetSpeed(REG_SP3, 0); //停止电机
                setSpd = 0;
                Motor_PID(0, 0, 1); //清除历史值
                break;
            }
            HAL_Delay(20);
        }

        // 告知安卓复位完成
        feedbackGoInit();
        //清除流程标志位
        flash_clear_used(1);
        if (!flash_read(&Process, 1)) //读取返回值为0 上次流程正常
        {
            Process = 0;
        }
    }
}

int dirSet(enum Side aimside, enum Side nowside)
{
    //初始速度设置
    if (aimside > nowside)
    {
        SetSpeed(REG_SP3, -ratedSpd);
        return -1;
    }
    else if (aimside < nowside)
    {
        SetSpeed(REG_SP3, ratedSpd);
        return 1;
    }
    return 0;
}

/**
  * 函数功能: 串口接收回调函数
  * 输入参数: UARTHandle:串口句柄
  */
char *end; // 用于函数 strtod(char*, char**);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    static uint8_t i = 0;
    if (UartHandle->Instance == husart3.Instance)
    {
        Rx_Buf[i] = husart3.Instance->DR;
        if (Rx_Buf[0] == ':')
        {
            i++;
            if (Rx_Buf[i - 1] == 0x0A)
            {
                if (Rx_Buf[i - 2] == 0x0D)
                {
                    Rx_Buf[i] = '\0';   // 手动添加结束符
                    UsartState = Ready; // 接收完成,通讯待机
                    i = 0;
                }
            }
        }
        else
            i = 0;
    }

    // 激光1
    if (UartHandle->Instance == UART4)
    {
        if (flag1_laser1 == 0)
        {
            if (aRxBuffer_laser1 == 'D')
            {
                flag1_laser1 = 1;
            }
        }
        else if (flag1_laser1 == 1 && flag2_laser1 == 0)
        {
            if (aRxBuffer_laser1 == '=')
            {
                flag2_laser1 = 1;
            }
            else
            {
                flag1_laser1 = 0;
            }
        }
        else if (flag1_laser1 == 1 && flag2_laser1 == 1 && aRxBuffer_laser1 == 'm')
        {
            flag1_laser1 = flag2_laser1 = 0;
            //更新前刷新记录历史值
            lastDistance2_L1 = lastDistance_L1;
            lastDistance_L1 = distance_laser1;
            distance_laser1 = strtod((const char *)buf_laser1, &end);
            // if (distance_L1 == 0)
            //   distance_L1 = lastDistance_L1;
            memset(buf_laser1, 0, 10);
            count_laser1 = 0;
        }
        else if (count_laser1 > 5)
        {
            count_laser1 = flag1_laser1 = flag2_laser1 = 0;
        }
        else
        {
            buf_laser1[count_laser1++] = aRxBuffer_laser1;
        }
        HAL_UART_Receive_IT(husart_laser1, &aRxBuffer_laser1, 1);
    }

    if (UartHandle->Instance == USART2)
    {
        if (flag1_laser2 == 0)
        {
            if (aRxBuffer_laser2 == 'D')
            {
                flag1_laser2 = 1;
            }
        }
        else if (flag1_laser2 == 1 && flag2_laser2 == 0)
        {
            if (aRxBuffer_laser2 == '=')
            {
                flag2_laser2 = 1;
            }
            else
            {
                flag1_laser2 = 0;
            }
        }
        else if (flag1_laser2 == 1 && flag2_laser2 == 1 && aRxBuffer_laser2 == 'm')
        {
            flag1_laser2 = flag2_laser2 = 0;
            //更新前刷新记录历史值
            lastDistance2_L2 = lastDistance_L2;
            lastDistance_L2 = distance_laser2;
            distance_laser2 = strtod((const char *)buf_laser2, &end);
            memset(buf_laser2, 0, 10);
            count_laser2 = 0;
        }
        else if (count_laser2 > 5)
        {
            count_laser2 = flag1_laser2 = flag2_laser2 = 0;
        }
        else
        {
            buf_laser2[count_laser2++] = aRxBuffer_laser2;
        }
        HAL_UART_Receive_IT(husart_laser2, &aRxBuffer_laser2, 1);
    }
}
