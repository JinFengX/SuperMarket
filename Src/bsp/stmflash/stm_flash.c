/**
  ******************************************************************************
  * 文件名程: stm_flash.c 
  * 作    者: 硬石嵌入式开发团队
  * 版    本: V1.0
  * 编写日期: 2015-10-04
  * 功    能: 内部Falsh读写实现
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
#include "stmflash/stm_flash.h"
#include <string.h>

/* 私有类型定义 --------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
#if STM32_FLASH_SIZE < 256
#define STM_SECTOR_SIZE 1024 //字节
#else
#define STM_SECTOR_SIZE 2048
#endif

/* 私有变量 ------------------------------------------------------------------*/
#if STM32_FLASH_WREN                               //如果使能了写
static uint16_t STMFLASH_BUF[STM_SECTOR_SIZE / 2]; //最多是2K字节
static FLASH_EraseInitTypeDef EraseInitStruct;
#endif

/* 扩展变量 ------------------------------------------------------------------*/
/* 私有函数原形 --------------------------------------------------------------*/
/* 函数体 --------------------------------------------------------------------*/
/**
  * 函数功能: 读取指定地址的半字(16位数据)
  * 输入参数: faddr:读地址(此地址必须为2的倍数!!)
  * 返 回 值: 返回值:对应数据.
  * 说    明：无
  */
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr)
{
    return *(__IO uint16_t *)faddr;
}

#if STM32_FLASH_WREN //如果使能了写
/**
  * 函数功能: 不检查的写入
  * 输入参数: WriteAddr:起始地址
  *           pBuffer:数据指针
  *           NumToWrite:半字(16位)数
  * 返 回 值: 无
  * 说    明：无
  */
void STMFLASH_Write_NoCheck(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t NumToWrite)
{
    uint16_t i;

    for (i = 0; i < NumToWrite; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, WriteAddr, pBuffer[i]);
        WriteAddr += 2; //地址增加2.
    }
}

/**
  * 函数功能: 从指定地址开始写入指定长度的数据
  * 输入参数: WriteAddr:起始地址(此地址必须为2的倍数!!)
  *           pBuffer:数据指针
  *           NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
  * 返 回 值: 无
  * 说    明：无
  */
void STMFLASH_Write(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t NumToWrite)
{
    uint32_t SECTORError = 0;
    uint16_t secoff;    //扇区内偏移地址(16位字计算)
    uint16_t secremain; //扇区内剩余地址(16位字计算)
    uint16_t i;
    uint32_t secpos;  //扇区地址
    uint32_t offaddr; //去掉0X08000000后的地址

    if (WriteAddr < FLASH_BASE || (WriteAddr >= (FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
        return; //非法地址

    HAL_FLASH_Unlock(); //解锁

    offaddr = WriteAddr - FLASH_BASE;         //实际偏移地址.
    secpos = offaddr / STM_SECTOR_SIZE;       //扇区地址  0~127 for STM32F103RBT6
    secoff = (offaddr % STM_SECTOR_SIZE) / 2; //在扇区内的偏移(2个字节为基本单位.)
    secremain = STM_SECTOR_SIZE / 2 - secoff; //扇区剩余空间大小
    if (NumToWrite <= secremain)
        secremain = NumToWrite; //不大于该扇区范围

    while (1)
    {
        STMFLASH_Read(secpos * STM_SECTOR_SIZE + FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2); //读出整个扇区的内容
        for (i = 0; i < secremain; i++)                                                          //校验数据
        {
            if (STMFLASH_BUF[secoff + i] != 0XFFFF)
                break; //需要擦除
        }
        if (i < secremain) //需要擦除
        {
            //擦除这个扇区
            /* Fill EraseInit structure*/

            // ////////////////////////
            // static uint16_t rx[1024];
            // memset(rx, 0, 2048);
            // EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
            // EraseInitStruct.PageAddress = FLASH_START_ADDRESS;
            // EraseInitStruct.NbPages = 1;
            // HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
            // STMFLASH_Read(secpos * STM_SECTOR_SIZE + FLASH_BASE, rx, 1024);
            // memset(rx, 0, 2048);
            // STMFLASH_Read(FLASH_START_ADDRESS, rx, 1024);
            // ////////////////////////

            EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
            EraseInitStruct.PageAddress = secpos * STM_SECTOR_SIZE + FLASH_BASE;
            EraseInitStruct.NbPages = 1;
            HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);

            // /////////////////////
            // STMFLASH_Read(secpos * STM_SECTOR_SIZE + FLASH_BASE, rx, 1024);
            // memset(rx, 0, 2048);
            // STMFLASH_Read(FLASH_START_ADDRESS, rx, 1024);
            // //////////////////////

            for (i = 0; i < secremain; i++) //复制
            {
                STMFLASH_BUF[i + secoff] = pBuffer[i];
            }
            STMFLASH_Write_NoCheck(secpos * STM_SECTOR_SIZE + FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2); //写入整个扇区
        }
        else
        {
            STMFLASH_Write_NoCheck(WriteAddr, pBuffer, secremain); //写已经擦除了的,直接写入扇区剩余区间.
        }
        if (NumToWrite == secremain)
            break; //写入结束了
        else       //写入未结束
        {
            secpos++;                //扇区地址增1
            secoff = 0;              //偏移位置为0
            pBuffer += secremain;    //指针偏移
            WriteAddr += secremain;  //写地址偏移
            NumToWrite -= secremain; //字节(16位)数递减
            if (NumToWrite > (STM_SECTOR_SIZE / 2))
                secremain = STM_SECTOR_SIZE / 2; //下一个扇区还是写不完
            else
                secremain = NumToWrite; //下一个扇区可以写完了
        }
    };
    HAL_FLASH_Lock(); //上锁
}
#endif

/**
  * 函数功能: 从指定地址开始读出指定长度的数据
  * 输入参数: ReadAddr:起始地址
  *           pBuffer:数据指针
  *           NumToRead:半字(16位)数
  * 返 回 值: 无
  * 说    明：无
  */
void STMFLASH_Read(uint32_t ReadAddr, uint16_t *pBuffer, uint16_t NumToRead)
{
    uint16_t i;

    for (i = 0; i < NumToRead; i++)
    {
        pBuffer[i] = STMFLASH_ReadHalfWord(ReadAddr); //读取2个字节.
        ReadAddr += 2;                                //偏移2个字节.
    }
}

//------------------以下为均衡磨损--------------------//
/*
    |0x00|0x00|
    |0x00|0x00|  // 已用空间

    |0x5A|data|
    |0x5A|data|  // 有效数据
    
    |0xff|0xff|  // 未用空间
    |0xff|0xff|
    |0xff|0xff|
*/
// 在未用的空间处写入新数据
uint8_t flash_write_new(uint8_t *pBuffer, uint16_t NumToWrite)
{
    uint32_t addr = FLASH_START_ADDRESS;
    // uint32_t addr_used = FLASH_START_ADDRESS;
    uint16_t write_buf[STM_SECTOR_SIZE / 2] = {0};
    int i;
    uint32_t SECTORError = 0;

    // 要写的数据多于页大小
    if (NumToWrite > STM_SECTOR_SIZE / 2)
    {
        return 0;
    }

    // // 找到第一块已用的半字
    // while (addr_used < FLASH_END_ADDRESS && STMFLASH_ReadHalfWord(addr_used) == 0)
    // {
    //     addr_used += 2;
    // }

    // 找到第一块未用的半字
    while (addr < FLASH_END_ADDRESS && STMFLASH_ReadHalfWord(addr) != 0xffff)
    {
        addr += 2;
    }

    // 如果没有可用的空间 或 剩余可用的空间不足够大 -> 擦除页
    if (addr >= FLASH_END_ADDRESS || (FLASH_END_ADDRESS - addr) / 2 < NumToWrite)
    {
        HAL_FLASH_Unlock();
        EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
        EraseInitStruct.PageAddress = FLASH_START_ADDRESS;
        EraseInitStruct.NbPages = 1;
        HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
        HAL_FLASH_Lock();
        addr = FLASH_START_ADDRESS;
    }

    // // 清空上次的有效数据
    // if (addr > addr_used)
    // {
    //     HAL_FLASH_Unlock();
    //     STMFLASH_Write_NoCheck(addr_used, write_buf, (addr - addr_used) / 2);
    //     HAL_FLASH_Lock();
    // }

    for (i = 0; i < NumToWrite; i++)
    {
        memset((uint8_t *)(write_buf + i) + 1, 0x5A, 1);
        memcpy((uint8_t *)(write_buf + i), pBuffer + i, 1);
    }

    HAL_FLASH_Unlock();
    STMFLASH_Write_NoCheck(addr, write_buf, NumToWrite);
    HAL_FLASH_Lock();
    return 1;
}

// 清除NumToClear个有效数据
uint8_t flash_clear_used(uint16_t NumToClear)
{
    uint32_t addr = FLASH_START_ADDRESS;
    uint16_t write_buf[STM_SECTOR_SIZE / 2] = {0};

    // 找到第一块未用的半字
    while (addr < FLASH_END_ADDRESS && STMFLASH_ReadHalfWord(addr) != 0xffff)
    {
        addr += 2;
    }

    // 如果没找到  说明刚好用完
    //(似乎不要这个if也行)
    if (addr >= FLASH_END_ADDRESS)
    {
        addr = FLASH_END_ADDRESS;
    }

    // 已用的空间都没有NumToClear个
    if ((addr - FLASH_START_ADDRESS) / 2 < NumToClear)
    {
        NumToClear = (addr - FLASH_START_ADDRESS) / 2;
        addr = FLASH_START_ADDRESS;
    }
    else
    {
        addr -= NumToClear * 2;
    }

    HAL_FLASH_Unlock();
    STMFLASH_Write_NoCheck(addr, write_buf, NumToClear);
    HAL_FLASH_Lock();
    return 1;
}

// 清除所有有效数据
uint8_t flash_clear_all_used()
{
    uint32_t addr = FLASH_START_ADDRESS;
    uint16_t write_buf[STM_SECTOR_SIZE / 2] = {0};
    uint16_t NumToClear;

    // 找到第一块未用的半字
    while (addr < FLASH_END_ADDRESS && STMFLASH_ReadHalfWord(addr) != 0xffff)
    {
        addr += 2;
    }

    // 如果没找到  说明刚好用完
    if (addr >= FLASH_END_ADDRESS)
    {
        addr = FLASH_END_ADDRESS;
    }

    NumToClear = (addr - FLASH_START_ADDRESS) / 2;

    HAL_FLASH_Unlock();
    STMFLASH_Write_NoCheck(FLASH_START_ADDRESS, write_buf, NumToClear);
    HAL_FLASH_Lock();
    return 1;
}

// 读数据
// 如果有效数据不足NumToRead个，返回0
uint8_t flash_read(uint8_t *pBuffer, uint16_t NumToRead)
{
    uint32_t addr = FLASH_START_ADDRESS;
    uint16_t read_buf[STM_SECTOR_SIZE / 2];
    int i;

    // 要写的数据多于页大小
    if (NumToRead > STM_SECTOR_SIZE / 2)
    {
        return 0;
    }

    // 找到第一块未用的半字
    while (addr < FLASH_END_ADDRESS && STMFLASH_ReadHalfWord(addr) != 0xffff)
    {
        addr += 2;
    }

    // 如果没找到  说明刚好用完
    if (addr >= FLASH_END_ADDRESS)
    {
        addr = FLASH_END_ADDRESS;
    }

    // 已用的空间都没有NumToWrite个
    if ((addr - FLASH_START_ADDRESS) / 2 < NumToRead)
    {
        return 0;
    }

    addr -= NumToRead * 2;

    // 上次未写入NumToWrite个
    if ((STMFLASH_ReadHalfWord(addr) >> 8) != 0x5A)
    {
        return 0;
    }

    STMFLASH_Read(addr, read_buf, NumToRead);

    // 清除标志位0x5a
    for (i = 0; i < NumToRead; i++)
    {
        pBuffer[i] = read_buf[i] % 1024;
    }

    return 1;
}

//////////////弃用///////////////////
// uint8_t flash_write_used(uint8_t *pBuffer, uint16_t NumToWrite)
// {
//     uint32_t addr = FLASH_START_ADDRESS;
//     uint16_t write_buf[STM_SECTOR_SIZE / 2];
//     int i;

//     // 要写的数据多于页大小
//     if (NumToWrite > STM_SECTOR_SIZE / 2)
//     {
//         return 0;
//     }

//     // 找到第一块未用的半字
//     while (addr < FLASH_END_ADDRESS && STMFLASH_ReadHalfWord(addr) != 0xffff)
//     {
//         addr += 2;
//     }

//     // 如果没找到  说明刚好用完
//     if (addr >= FLASH_END_ADDRESS)
//     {
//         addr = FLASH_END_ADDRESS;
//     }

//     // 已用的空间都没有NumToWrite个
//     if ((addr - FLASH_START_ADDRESS) / 2 < NumToWrite)
//     {
//         return 0;
//     }

//     addr -= NumToWrite * 2;

//     // 上次未写入NumToWrite个
//     if ((STMFLASH_ReadHalfWord(addr) >> 8) != 0x5A)
//     {
//         return 0;
//     }

//     for (i = 0; i < NumToWrite; i++)
//     {
//         memset((uint8_t *)(write_buf + i) + 1, 0x5A, 1);
//         memcpy((uint8_t *)(write_buf + i), pBuffer + i, 1);
//     }

//     HAL_FLASH_Unlock();
//     STMFLASH_Write_NoCheck(addr, write_buf, NumToWrite);
//     HAL_FLASH_Lock();
//     return 1;
// }
/******************* (C) COPYRIGHT 2015-2020 硬石嵌入式开发团队 *****END OF FILE****/
