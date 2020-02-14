/**
  ******************************************************************************
  * 文件名程: bsp_exSRAM.c 
  * 作    者: 硬石嵌入式开发团队
  * 版    本: V1.0
  * 编写日期: 2015-10-04
  * 功    能: 液晶底层驱动函数实现
  ******************************************************************************
  * 说明：
  * 本例程配套硬石stm32开发板YS-F1Pro使用。
  * 
  * 淘宝：
  * 论坛：http://www.ing10bbs.com
  * 版权归硬石嵌入式开发团队所有，请勿商用。
  ******************************************************************************
  */
/* 包含头文件 ----------------------------------------------------------------*/
#include "headfile.h"

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
/* 私有变量 ------------------------------------------------------------------*/
SRAM_HandleTypeDef hexSRAM;
static int FSMC_exSRAM_Initialized = 0;
static int FSMC_exSRAM_DeInitialized = 0;

/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/
/**
  * 函数功能: 初始化扩展SRAM
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：无
  */
void MX_FSMC_exSRAM_Init(void)
{
  FSMC_NORSRAM_TimingTypeDef Timing;

  /* FSMC配置 */
  hexSRAM.Instance = FSMC_NORSRAM_DEVICE;
  hexSRAM.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hexSRAM.Init */
  hexSRAM.Init.NSBank = FSMC_NORSRAM_BANK3; // 使用NE3
  hexSRAM.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hexSRAM.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hexSRAM.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;//存储器数据宽度为16bit  
  hexSRAM.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hexSRAM.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hexSRAM.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hexSRAM.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hexSRAM.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE; //存储器写使能 
  hexSRAM.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hexSRAM.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;// 读写使用相同的时序
  hexSRAM.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hexSRAM.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  /* FSMC操作时间 */
  Timing.AddressSetupTime = 0x00; //地址建立时间（ADDSET）
  Timing.AddressHoldTime = 0x00;  //地址保持时间（ADDHLD）模式A未用到
  Timing.DataSetupTime = 0x03;    //数据保持时间（DATAST）为3个HCLK 4/72M=55ns
  Timing.BusTurnAroundDuration = 0x00;
  Timing.CLKDivision = 0x00;
  Timing.DataLatency = 0x00;
  Timing.AccessMode = FSMC_ACCESS_MODE_A; //模式A
  HAL_SRAM_Init(&hexSRAM, &Timing, &Timing);//读写同样时序

  __HAL_AFIO_FSMCNADV_DISCONNECTED();

}

/**
  * 函数功能: 初始化扩展SRAM的IO引脚
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：该函数被HAL_SRAM_MspInit函数调用
  */
static void HAL_FSMC_exSRAM_MspInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  /* 如果已经完成初始化就无需初始化第二遍 */
  if (FSMC_exSRAM_Initialized)
  {
    return;
  }
  FSMC_exSRAM_Initialized = 1;
  
  
  /* 使能相关端口时钟 */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  
  /* 使能FSMC外设时钟 */
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
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; /* 复用推挽输出模式 */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;/* 高速 */
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; /* 复用推挽输出模式 */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;/* 高速 */
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10 
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; /* 复用推挽输出模式 */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;/* 高速 */
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15|GPIO_PIN_0 
                          |GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP; /* 复用推挽输出模式 */
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;/* 高速 */
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

/**
  * 函数功能: 初始化FSMC的IO引脚
  * 输入参数: hsram：SRAM外设句柄指针
  * 返 回 值: 无
  * 说    明：该函数被HAL库内部函数调用
  */	
void HAL_SRAM_MspInit(SRAM_HandleTypeDef* hsram)
{
  /* 初始化扩展SRAM的IO引脚 */
  HAL_FSMC_exSRAM_MspInit();
}

/**
  * 函数功能: 反初始化扩展SRAM的IO引脚
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：该函数被HAL_SRAM_MspDeInit函数调用
  */
static void HAL_FSMC_exSRAM_MspDeInit(void)
{
  /* 如果已经完成反初始化就无需初始化第二遍 */
  if (FSMC_exSRAM_DeInitialized) {
    return;
  }
  FSMC_exSRAM_DeInitialized = 1;
  
  /* 禁用FSMC外设时钟 */
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
  * 函数功能: 反初始化FSMC的IO引脚
  * 输入参数: hsram：SRAM外设句柄指针
  * 返 回 值: 无
  * 说    明：该函数被HAL库内部函数调用
  */	
void HAL_SRAM_MspDeInit(SRAM_HandleTypeDef* hsram)
{
  HAL_FSMC_exSRAM_MspDeInit();
}
/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
