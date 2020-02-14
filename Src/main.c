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
/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
__IO uint8_t Rx_Buf[50]; //�������ݻ���
uint8_t aRxBuffer_laser1;
uint8_t aRxBuffer_laser2;

/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/
/**
  * ��������: ϵͳʱ������
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE; // �ⲿ����8MHz
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9; // 9��Ƶ���õ�72MHz��ʱ��
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // ϵͳʱ�ӣ�72MHz
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;        // AHBʱ�ӣ�72MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;         // APB1ʱ�ӣ�36MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;         // APB2ʱ�ӣ�72MHz
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

    // HAL_RCC_GetHCLKFreq()/1000    1ms�ж�һ��
    // HAL_RCC_GetHCLKFreq()/100000	 10us�ж�һ��
    // HAL_RCC_GetHCLKFreq()/1000000 1us�ж�һ��
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000); // ���ò�����ϵͳ�δ�ʱ��
    /* ϵͳ�δ�ʱ��ʱ��Դ */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    /* ϵͳ�δ�ʱ���ж����ȼ����� */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/**
  * ��������: ������.
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ��: ��
  */
uint8_t test;

//�趨�ٶ�Ϊ��ֵʱ������������ת����˽�������ָ�������Ϊ˳ʱ�룬���յ�Ϊ��������ĸ�����
//��������Ϊ��ǰ�浽���յ�ľ���
enum Side //������
{
    A, //��λ��������
    B, //������������
    C  //ȡ����������
};
#define initDistance 0.6      //��ʼ����
#define outDistance 1.1       //��������
const enum Side initSide = A; //��ʼ��
enum Side nowSide = A;        //��ʼ��Ϊ��λ��������
enum Side aimSide = A;        //��ʼ��Ŀ����Ϊ��λ��������
uint8_t sideChange = 0;
int dir = 0;                  //ǰ������
#define filter 0.2            //ƽ��ϵ����ϵ��Խ��Խƽ��
float aimDistance = 0;        //Ŀ�꼤�����
float distance_L1 = 0.0;      //��ǰ�������
float lastDistance_L1 = 0.0;  //�ϴμ����ȡ����
float lastDistance2_L1 = 0.0; //���ϴμ����ȡ����
float distance_L2 = 0.0;      //��ǰ�������
float lastDistance_L2 = 0.0;  //�ϴμ����ȡ����
float lastDistance2_L2 = 0.0; //���ϴμ����ȡ����
uint8_t distanceMin = 0;      //��̾����־λ
float setSpd = 0;             //�趨�ٶ�
float height = 0;             //�߶�
float width = 0;              //���

const float AtoBLenth = 1.85;
const float BtoCLenth = 2.1;
const float CtoBLenth = 0.6;
const float BtoALenth = 0.5;

const float crtDistance = 0.06;  //������Χ�����������Χʹ��λ�á����ٶȴ������ƣ�����ʹ�õ��ٶȱջ�����
const float allowedError = 0.02; //������Χ����λ��

uint8_t Process = 0x00; //���̱�־

int dirSet(enum Side aimside, enum Side nowside);

int main(void)
{
    // HAL_Init();
    // /* ����ϵͳʱ�� */
    // SystemClock_Config();

    // /* ��ʼ�����ڲ����ô����ж����ȼ� */
    // MX_USARTx_Init();
    // my_mem_init(SRAMIN); //��ʼ���ڲ��ڴ��

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

    /* ��λ�������裬��ʼ��Flash�ӿں�ϵͳ�δ�ʱ�� */
    HAL_Init();
    /* ����ϵͳʱ�� */
    SystemClock_Config();

    /* ��ʼ�����ڲ����ô����ж����ȼ� */
    MX_USARTx_Init();

    // wifi��ʼ��
    my_mem_init(SRAMIN); //��ʼ���ڲ��ڴ��
    wifi_init();
    HAL_Delay(1000);

    // ��׿��������id
    send_ID();

    if (!flash_read(&Process, 1)) //��ȡ����ֵΪ0 ��������
    {
        Process = 0;
    }

    if (Process != 0x00) //�ϴ�δ������ɶ������쳣δ���� �����쳣��������
    {
        //����ϴ�δ��ɱ�־���쳣��־ д���쳣�����־
        flash_clear_used(1);
        Process = 0x01;
        flash_write_new(&Process, 1);

        //֪ͨ������쳣

        while (Process) //�쳣����
        {
            //�ȴ������֪ͨ�쳣�������
            HAL_Delay(3000);
            HAL_Delay(3000);

            //�쳣���
            flash_clear_used(1);
            if (!flash_read(&Process, 1)) //��ȡ����ֵΪ0ʱ ˵���쳣�Ѿ�ȷ�����
            {
                Process = 0;
            }//ProcessΪ0 ����ѭ��������������
        }
    }

    // ���հ�׿��״̬��ѯָ��
    wifiMsg_Parse(1);
    feedBackTypeGot(1);

    while (rec_repeat(&huart5, "CRdyOK@")) //�ȴ�ȡ����Ԫ��ʼ�����
        ;

    // �����ʼ��
    LASER_Init();
    HAL_UART_Receive_IT(husart_laser1, &aRxBuffer_laser1, 1); // ����1�Ĵ��� ���ж�
    HAL_Delay(500);
    // HAL_UART_Receive_IT(husart_laser2, &aRxBuffer_laser2, 1); // ����2�Ĵ��� ���ж�
    // HAL_Delay(500);

    //�����ʼ��
    HAL_UART_Receive_DMA(&husart3, (uint8_t *)&test, 1); // Data Direction: 485 --> USART1
    /* Disable the Half transfer complete interrupt */
    __HAL_DMA_DISABLE_IT(&hdma_rs485_rx, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(&hdma_rs485_rx, DMA_IT_TE);
    /* ��ʼ��ASDA-B2����,����Ϊ�ٶ�λ��ģʽ*/
    ASDAB2_Init();
    /* ����SP3�ٶ�ֵΪ600*0.1r/min  60r/min */
    SetSpeed(REG_SP3, 0);
    /* �����ŷ� */
    StartServo();
    HAL_Delay(2000); //���������

    // ��֪��׿���ص�״̬
    checkSysState();

    // while (1)
    // {
    //   height = 1.0;
    //   width = 0.5;
    //   uint8_t Dat[20] = {0};
    //   //���͸߶�
    //   sprintf(Dat, "DH%.5f!", height);
    //   while (send_cmd(&huart5, Dat, Dat, 1000)) //�ȴ��ظ�
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
        // ���հ�׿�����Ļ���λ����Ϣ
        wifiMsg_Parse(3);
        feedBackTypeGot(3);

        //*****************************************��ʼ��Ϣ��ȡ
        nowSide = A;
        aimSide = goodsLocation.side - 'A';
        aimDistance = goodsLocation.distance;
        // while(1);/////////////////////

        // ��֪��׿��ʼȡ��
        feedbackStartGetGoods();
        //����״̬д��flash
        Process = 0xFE;
        flash_write_new(&Process, 1);

        //******************************************ǰ��ȡ�����ʼ�ٶ��趨
        while (1)
        {
            dir = dirSet(aimSide, nowSide); //��ʱ��-1 ˳ʱ��1
            break;
        }

        //*******************************************�������ж�
        while (1)
        {
            if (aimSide == nowSide)
            {
                sideChange = 0;
                HAL_Delay(200);
                break; //������һ��
            }

            if (sideChange == 1) //�����δ����Ŀ������3���ڲ��ظ��ж�
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                sideChange = 0;
            }

            if ((dir < 0) && //��ʱ��
                ((nowSide == A) && (distance_laser1 > AtoBLenth) ||
                 (nowSide == B) && (distance_laser1 > BtoCLenth))) //2��þ���С����С�߳����ж�Ϊ������������
            {

                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                nowSide++;
                sideChange = 1;
            }

            if ((dir > 0) && //˳ʱ��
                ((nowSide == C) && (distance_laser1 < CtoBLenth) ||
                 (nowSide == B) && (distance_laser1 < BtoALenth))) //1��þ���С����С�߳����ж�Ϊ����˳������
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                nowSide--;
                sideChange = 1;
            }
        }

        //******************************************ȡ�����������
        while (1)
        {
            distance_L1 = distance_laser1 * (1 - filter) + lastDistance_L1 * filter;
            if ((distance_L1 - aimDistance) > crtDistance) //���Ʋ���ѡ��
                setSpd = ratedSpd;
            else if ((distance_L1 - aimDistance) < (-1 * crtDistance)) //���Ʋ���ѡ��
                setSpd = -1 * ratedSpd;
            else if (fabs(distance_L1 - aimDistance) <= crtDistance)
                setSpd = Motor_PID(aimDistance, distance_L1, 0);
            SetSpeed(REG_SP3, setSpd);

            if ((fabs(distance_laser1 - aimDistance) < allowedError)) //&&
                                                                      // (fabs(lastDistance_L1 - aimDistance) < allowedError) &&
                                                                      // (fabs(lastDistance2_L1 - aimDistance) < allowedError)) //������֡������ΧҪ�������һ��
            {
                SetSpeed(REG_SP3, 0); //ֹͣ���
                setSpd = 0;
                Motor_PID(0, 0, 1); //�����ʷֵ
                break;
            }
            HAL_Delay(20);
        }
        HAL_Delay(666);

        //***********************���ͻ���߶�����Լ�ȡ������
        height = goodsLocation.height;
        width = goodsLocation.width;
        uint8_t Dat[20] = {0};
        //���͸߶�
        sprintf(Dat, "DH%.5f!", height);
        while (send_cmd(&huart5, Dat, Dat, 100)) //�ȴ��ظ�
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

        while (rec_repeat(&huart5, "CGetOK@")) //�ȴ�ȡ�����
            ;

        // ��֪��׿ȡ�����
        GotGoodsResult = 0;
        feedbackGotGoodsResult();

        //***************************************ȡ��������ǰ��������
        aimSide = B;
        aimDistance = 1.8;
        //***************************************ǰ���������ʼ�ٶ��趨
        while (1)
        {
            dir = dirSet(aimSide, nowSide);
            break;
        }

        //*******************************************�������ж�
        while (1)
        {
            if (aimSide == nowSide)
            {
                sideChange = 0;
                HAL_Delay(200);
                break; //������һ��
            }

            if (sideChange == 1) //�����δ����Ŀ������3���ڲ��ظ��ж�
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                sideChange = 0;
            }

            if ((dir < 0) && //��ʱ��
                ((nowSide == A) && (distance_laser1 > AtoBLenth) ||
                 (nowSide == B) && (distance_laser1 > BtoCLenth))) //2��þ���С����С�߳����ж�Ϊ������������
            {

                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(500);
                nowSide++;
                sideChange = 1;
            }

            if ((dir > 0) && //˳ʱ��
                ((nowSide == C) && (distance_laser1 < CtoBLenth) ||
                 (nowSide == B) && (distance_laser1 < BtoALenth))) //1��þ���С����С�߳����ж�Ϊ����˳������
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(500);
                nowSide--;
                sideChange = 1;
            }
        }

        //***********************************������λ�ý���
        while (1)
        {
            distance_L1 = distance_laser1 * (1 - filter) + lastDistance_L1 * filter;
            if ((distance_L1 - aimDistance) > crtDistance) //���Ʋ���ѡ��
                setSpd = ratedSpd;
            else if ((distance_L1 - aimDistance) < (-1 * crtDistance)) //���Ʋ���ѡ��
                setSpd = -ratedSpd;
            else if (fabs(distance_L1 - aimDistance) <= crtDistance)
                setSpd = Motor_PID(aimDistance, distance_L1, 0);
            SetSpeed(REG_SP3, setSpd);

            if ((fabs(distance_laser1 - aimDistance) < allowedError)) //&&
                                                                      // (fabs(lastDistance_L1 - aimDistance) < allowedError) &&
                                                                      // (fabs(lastDistance2_L1 - aimDistance) < allowedError)) //������֡������ΧҪ�������һ��
            {
                SetSpeed(REG_SP3, 0); //ֹͣ���
                setSpd = 0;
                Motor_PID(0, 0, 1); //�����ʷֵ
                break;
            }
            HAL_Delay(20);
        }
        HAL_Delay(666);

        //*******************************����ж������
        while (send_cmd(&huart5, "CPut@", "CPut@", 100))
            ;

        while (rec_repeat(&huart5, "CPutOK@")) //�ȴ�ж�����
            ;

        // ��֪��׿�������
        LoseGoodsResult = 0;
        feedbackLoseGoodsResult();

        //***************************************����������ǰ��ж�̵�
        aimSide = B;
        aimDistance = 0.8;
        //***************************************ǰ��ж�̵��ʼ�ٶ��趨
        while (1)
        {
            dir = dirSet(aimSide, nowSide);
            break;
        }

        //*******************************************�������ж�
        while (1)
        {
            if (aimSide == nowSide)
            {
                sideChange = 0;
                HAL_Delay(200);
                break; //������һ��
            }

            if (sideChange == 1) //�����δ����Ŀ������3���ڲ��ظ��ж�
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                sideChange = 0;
            }

            if ((dir < 0) && //��ʱ��
                ((nowSide == A) && (distance_laser1 > AtoBLenth) ||
                 (nowSide == B) && (distance_laser1 > BtoCLenth))) //2��þ���С����С�߳����ж�Ϊ������������
            {

                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                nowSide++;
                sideChange = 1;
            }

            if ((dir > 0) && //˳ʱ��
                ((nowSide == C) && (distance_laser1 < CtoBLenth) ||
                 (nowSide == B) && (distance_laser1 < BtoALenth))) //1��þ���С����С�߳����ж�Ϊ����˳������
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                nowSide--;
                sideChange = 1;
            }
        }

        //***********************************λ�ý���
        while (1)
        {
            distance_L1 = distance_laser1 * (1 - filter) + lastDistance_L1 * filter;
            if ((distance_L1 - aimDistance) > crtDistance) //���Ʋ���ѡ��
                setSpd = ratedSpd;
            else if ((distance_L1 - aimDistance) < (-1 * crtDistance)) //���Ʋ���ѡ��
                setSpd = -ratedSpd;
            else if (fabs(distance_L1 - aimDistance) <= crtDistance)
                setSpd = Motor_PID(aimDistance, distance_L1, 0);
            SetSpeed(REG_SP3, setSpd);

            if ((fabs(distance_laser1 - aimDistance) < allowedError)) //&&
                                                                      // (fabs(lastDistance_L1 - aimDistance) < allowedError) &&
                                                                      // (fabs(lastDistance2_L1 - aimDistance) < allowedError)) //������֡������ΧҪ�������һ��
            {
                SetSpeed(REG_SP3, 0); //ֹͣ���
                setSpd = 0;
                Motor_PID(0, 0, 1); //�����ʷֵ
                break;
            }
            HAL_Delay(20);
        }

        //******************����ж��������
        while (send_cmd(&huart5, "CThw@", "CThw@", 100))
            ;

        while (rec_repeat(&huart5, "CThwOK@")) //�ȴ�ж�������
            ;

        //********************************ж������ɻص���ʼ��
        aimSide = A;
        aimDistance = 0.8;

        //***************************************ǰ����λ���ʼ�ٶ��趨
        while (1)
        {
            dir = dirSet(aimSide, nowSide);
            break;
        }

        //*******************************************�������ж�
        while (1)
        {
            if (aimSide == nowSide)
            {
                sideChange = 0;
                HAL_Delay(200);
                break; //������һ��
            }

            if (sideChange == 1) //�����δ����Ŀ������3���ڲ��ظ��ж�
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                sideChange = 0;
            }

            if ((dir < 0) && //��ʱ��
                ((nowSide == A) && (distance_laser1 > AtoBLenth) ||
                 (nowSide == B) && (distance_laser1 > BtoCLenth))) //2��þ���С����С�߳����ж�Ϊ������������
            {

                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                nowSide++;
                sideChange = 1;
            }

            if ((dir > 0) && //˳ʱ��
                ((nowSide == C) && (distance_laser1 < CtoBLenth) ||
                 (nowSide == B) && (distance_laser1 < BtoALenth))) //1��þ���С����С�߳����ж�Ϊ����˳������
            {
                HAL_Delay(1000);
                HAL_Delay(1000);
                HAL_Delay(1000);
                nowSide--;
                sideChange = 1;
            }
        }

        //***********************************λ�ý���
        while (1)
        {
            distance_L1 = distance_laser1 * (1 - filter) + lastDistance_L1 * filter;
            if ((distance_L1 - aimDistance) > crtDistance) //���Ʋ���ѡ��
                setSpd = ratedSpd;
            else if ((distance_L1 - aimDistance) < (-1 * crtDistance)) //���Ʋ���ѡ��
                setSpd = -ratedSpd;
            else if (fabs(distance_L1 - aimDistance) <= crtDistance)
                setSpd = Motor_PID(aimDistance, distance_L1, 0);
            SetSpeed(REG_SP3, setSpd);

            if ((fabs(distance_laser1 - aimDistance) < allowedError)) //&&
                                                                      // (fabs(lastDistance_L1 - aimDistance) < allowedError) &&
                                                                      // (fabs(lastDistance2_L1 - aimDistance) < allowedError)) //������֡������ΧҪ�������һ��
            {
                SetSpeed(REG_SP3, 0); //ֹͣ���
                setSpd = 0;
                Motor_PID(0, 0, 1); //�����ʷֵ
                break;
            }
            HAL_Delay(20);
        }

        // ��֪��׿��λ���
        feedbackGoInit();
        //������̱�־λ
        flash_clear_used(1);
        if (!flash_read(&Process, 1)) //��ȡ����ֵΪ0 �ϴ���������
        {
            Process = 0;
        }
    }
}

int dirSet(enum Side aimside, enum Side nowside)
{
    //��ʼ�ٶ�����
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
  * ��������: ���ڽ��ջص�����
  * �������: UARTHandle:���ھ��
  */
char *end; // ���ں��� strtod(char*, char**);
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
                    Rx_Buf[i] = '\0';   // �ֶ���ӽ�����
                    UsartState = Ready; // �������,ͨѶ����
                    i = 0;
                }
            }
        }
        else
            i = 0;
    }

    // ����1
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
            //����ǰˢ�¼�¼��ʷֵ
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
            //����ǰˢ�¼�¼��ʷֵ
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
