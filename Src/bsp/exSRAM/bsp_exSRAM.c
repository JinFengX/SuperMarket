/**
  ******************************************************************************
  * �ļ�����: bsp_exSRAM.c 
  * ��    ��: ӲʯǶ��ʽ�����Ŷ�
  * ��    ��: V1.0
  * ��д����: 2015-10-04
  * ��    ��: Һ���ײ���������ʵ��
  ******************************************************************************
  * ˵����
  * ����������Ӳʯstm32������YS-F1Proʹ�á�
  * 
  * �Ա���
  * ��̳��http://www.ing10bbs.com
  * ��Ȩ��ӲʯǶ��ʽ�����Ŷ����У��������á�
  ******************************************************************************
  */
/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "headfile.h"

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
/* ˽�б��� ------------------------------------------------------------------*/
SRAM_HandleTypeDef hexSRAM;
static int FSMC_exSRAM_Initialized = 0;
static int FSMC_exSRAM_DeInitialized = 0;

/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/
/**
  * ��������: ��ʼ����չSRAM
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
void MX_FSMC_exSRAM_Init(void)
{
  FSMC_NORSRAM_TimingTypeDef Timing;

  /* FSMC���� */
  hexSRAM.Instance = FSMC_NORSRAM_DEVICE;
  hexSRAM.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hexSRAM.Init */
  hexSRAM.Init.NSBank = FSMC_NORSRAM_BANK3; // ʹ��NE3
  hexSRAM.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hexSRAM.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hexSRAM.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;//�洢�����ݿ��Ϊ16bit  
  hexSRAM.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hexSRAM.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hexSRAM.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hexSRAM.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hexSRAM.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE; //�洢��дʹ�� 
  hexSRAM.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hexSRAM.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;// ��дʹ����ͬ��ʱ��
  hexSRAM.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hexSRAM.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  /* FSMC����ʱ�� */
  Timing.AddressSetupTime = 0x00; //��ַ����ʱ�䣨ADDSET��
  Timing.AddressHoldTime = 0x00;  //��ַ����ʱ�䣨ADDHLD��ģʽAδ�õ�
  Timing.DataSetupTime = 0x03;    //���ݱ���ʱ�䣨DATAST��Ϊ3��HCLK 4/72M=55ns
  Timing.BusTurnAroundDuration = 0x00;
  Timing.CLKDivision = 0x00;
  Timing.DataLatency = 0x00;
  Timing.AccessMode = FSMC_ACCESS_MODE_A; //ģʽA
  HAL_SRAM_Init(&hexSRAM, &Timing, &Timing);//��дͬ��ʱ��

  __HAL_AFIO_FSMCNADV_DISCONNECTED();

}

/**
  * ��������: ��ʼ����չSRAM��IO����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    �����ú�����HAL_SRAM_MspInit��������
  */
static void HAL_FSMC_exSRAM_MspInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  /* ����Ѿ���ɳ�ʼ���������ʼ���ڶ��� */
  if (FSMC_exSRAM_Initialized)
  {
    return;
  }
  FSMC_exSRAM_Initialized = 1;
  
  
  /* ʹ����ض˿�ʱ�� */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  
  /* ʹ��FSMC����ʱ�� */
  __HAL_RCC_FSMC_CLK_ENABLE();
  
  /** FSMC GPIO Configuration  
  PF0   ------> FSMC_A0
  PF1   ------> FSMC_A1
  PF2   ------> FSMC_A2
  PF3   ------> FSMC_A3
  PF4   ------> FSMC_A4
  PF5   ------> FSMC_A5
  PF12   ------> FSMC_A6
  PF13   ------> FSMC_A7
  PF14   ------> FSMC_A8
  PF15   ------> FSMC_A9
  PG0   ------> FSMC_A10
  PG1   ------> FSMC_A11
  PE7   ------> FSMC_D4
  PE8   ------> FSMC_D5
  PE9   ------> FSMC_D6
  PE10   ------> FSMC_D7
  PE11   ------> FSMC_D8
  PE12   ------> FSMC_D9
  PE13   ------> FSMC_D10
  PE14   ------> FSMC_D11
  PE15   ------> FSMC_D12
  PD8   ------> FSMC_D13
  PD9   ------> FSMC_D14
  PD10   ------> FSMC_D15
  PD11   ------> FSMC_A16
  PD12   ------> FSMC_A17
  PD14   ------> FSMC_D0
  PD15   ------> FSMC_D1
  PG2   ------> FSMC_A12
  PG3   ------> FSMC_A13
  PG4   ------> FSMC_A14
  PG5   ------> FSMC_A15
  PD0   ------> FSMC_D2
  PD1   ------> FSMC_D3
  PD4   ------> FSMC_NOE
  PD5   ------> FSMC_NWE
  PG10   ------> FSMC_NE3
  PE0   ------> FSMC_NBL0
  PE1   ------> FSMC_NBL1
  */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_12|GPIO_PIN_13 
                          |GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; /* �����������ģʽ */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;/* ���� */
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; /* �����������ģʽ */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;/* ���� */
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10 
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; /* �����������ģʽ */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;/* ���� */
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0 
                          |GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; /* �����������ģʽ */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;/* ���� */
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/**
  * ��������: ��ʼ��FSMC��IO����
  * �������: hsram��SRAM������ָ��
  * �� �� ֵ: ��
  * ˵    �����ú�����HAL���ڲ���������
  */	
void HAL_SRAM_MspInit(SRAM_HandleTypeDef* hsram)
{
  /* ��ʼ����չSRAM��IO���� */
  HAL_FSMC_exSRAM_MspInit();
}

/**
  * ��������: ����ʼ����չSRAM��IO����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    �����ú�����HAL_SRAM_MspDeInit��������
  */
static void HAL_FSMC_exSRAM_MspDeInit(void)
{
  /* ����Ѿ���ɷ���ʼ���������ʼ���ڶ��� */
  if (FSMC_exSRAM_DeInitialized) {
    return;
  }
  FSMC_exSRAM_DeInitialized = 1;
  
  /* ����FSMC����ʱ�� */
  __HAL_RCC_FSMC_CLK_DISABLE();
  
  /** FSMC GPIO Configuration  
  PF0   ------> FSMC_A0
  PF1   ------> FSMC_A1
  PF2   ------> FSMC_A2
  PF3   ------> FSMC_A3
  PF4   ------> FSMC_A4
  PF5   ------> FSMC_A5
  PF12   ------> FSMC_A6
  PF13   ------> FSMC_A7
  PF14   ------> FSMC_A8
  PF15   ------> FSMC_A9
  PG0   ------> FSMC_A10
  PG1   ------> FSMC_A11
  PE7   ------> FSMC_D4
  PE8   ------> FSMC_D5
  PE9   ------> FSMC_D6
  PE10   ------> FSMC_D7
  PE11   ------> FSMC_D8
  PE12   ------> FSMC_D9
  PE13   ------> FSMC_D10
  PE14   ------> FSMC_D11
  PE15   ------> FSMC_D12
  PD8   ------> FSMC_D13
  PD9   ------> FSMC_D14
  PD10   ------> FSMC_D15
  PD11   ------> FSMC_A16
  PD12   ------> FSMC_A17
  PD14   ------> FSMC_D0
  PD15   ------> FSMC_D1
  PG2   ------> FSMC_A12
  PG3   ------> FSMC_A13
  PG4   ------> FSMC_A14
  PG5   ------> FSMC_A15
  PD0   ------> FSMC_D2
  PD1   ------> FSMC_D3
  PD4   ------> FSMC_NOE
  PD5   ------> FSMC_NWE
  PG10   ------> FSMC_NE3
  PE0   ------> FSMC_NBL0
  PE1   ------> FSMC_NBL1
  */

  HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_12|GPIO_PIN_13 
                          |GPIO_PIN_14|GPIO_PIN_15);

  HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_10);

  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10 
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1);

  HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0 
                          |GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5);
}

/**
  * ��������: ����ʼ��FSMC��IO����
  * �������: hsram��SRAM������ָ��
  * �� �� ֵ: ��
  * ˵    �����ú�����HAL���ڲ���������
  */	
void HAL_SRAM_MspDeInit(SRAM_HandleTypeDef* hsram)
{
  HAL_FSMC_exSRAM_MspDeInit();
}
/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
