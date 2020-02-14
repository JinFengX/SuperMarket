/**
  ******************************************************************************
  * �ļ�����: stm_flash.c 
  * ��    ��: ӲʯǶ��ʽ�����Ŷ�
  * ��    ��: V1.0
  * ��д����: 2015-10-04
  * ��    ��: �ڲ�Falsh��дʵ��
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
#include "stmflash/stm_flash.h"
#include <string.h>

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
#if STM32_FLASH_SIZE < 256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else
#define STM_SECTOR_SIZE 2048
#endif

/* ˽�б��� ------------------------------------------------------------------*/
#if STM32_FLASH_WREN                               //���ʹ����д
static uint16_t STMFLASH_BUF[STM_SECTOR_SIZE / 2]; //�����2K�ֽ�
static FLASH_EraseInitTypeDef EraseInitStruct;
#endif

/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
/* ������ --------------------------------------------------------------------*/
/**
  * ��������: ��ȡָ����ַ�İ���(16λ����)
  * �������: faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
  * �� �� ֵ: ����ֵ:��Ӧ����.
  * ˵    ������
  */
uint16_t STMFLASH_ReadHalfWord(uint32_t faddr)
{
    return *(__IO uint16_t *)faddr;
}

#if STM32_FLASH_WREN //���ʹ����д
/**
  * ��������: ������д��
  * �������: WriteAddr:��ʼ��ַ
  *           pBuffer:����ָ��
  *           NumToWrite:����(16λ)��
  * �� �� ֵ: ��
  * ˵    ������
  */
void STMFLASH_Write_NoCheck(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t NumToWrite)
{
    uint16_t i;

    for (i = 0; i < NumToWrite; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, WriteAddr, pBuffer[i]);
        WriteAddr += 2; //��ַ����2.
    }
}

/**
  * ��������: ��ָ����ַ��ʼд��ָ�����ȵ�����
  * �������: WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
  *           pBuffer:����ָ��
  *           NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
  * �� �� ֵ: ��
  * ˵    ������
  */
void STMFLASH_Write(uint32_t WriteAddr, uint16_t *pBuffer, uint16_t NumToWrite)
{
    uint32_t SECTORError = 0;
    uint16_t secoff;    //������ƫ�Ƶ�ַ(16λ�ּ���)
    uint16_t secremain; //������ʣ���ַ(16λ�ּ���)
    uint16_t i;
    uint32_t secpos;  //������ַ
    uint32_t offaddr; //ȥ��0X08000000��ĵ�ַ

    if (WriteAddr < FLASH_BASE || (WriteAddr >= (FLASH_BASE + 1024 * STM32_FLASH_SIZE)))
        return; //�Ƿ���ַ

    HAL_FLASH_Unlock(); //����

    offaddr = WriteAddr - FLASH_BASE;         //ʵ��ƫ�Ƶ�ַ.
    secpos = offaddr / STM_SECTOR_SIZE;       //������ַ  0~127 for STM32F103RBT6
    secoff = (offaddr % STM_SECTOR_SIZE) / 2; //�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
    secremain = STM_SECTOR_SIZE / 2 - secoff; //����ʣ��ռ��С
    if (NumToWrite <= secremain)
        secremain = NumToWrite; //�����ڸ�������Χ

    while (1)
    {
        STMFLASH_Read(secpos * STM_SECTOR_SIZE + FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2); //������������������
        for (i = 0; i < secremain; i++)                                                          //У������
        {
            if (STMFLASH_BUF[secoff + i] != 0XFFFF)
                break; //��Ҫ����
        }
        if (i < secremain) //��Ҫ����
        {
            //�����������
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

            for (i = 0; i < secremain; i++) //����
            {
                STMFLASH_BUF[i + secoff] = pBuffer[i];
            }
            STMFLASH_Write_NoCheck(secpos * STM_SECTOR_SIZE + FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2); //д����������
        }
        else
        {
            STMFLASH_Write_NoCheck(WriteAddr, pBuffer, secremain); //д�Ѿ������˵�,ֱ��д������ʣ������.
        }
        if (NumToWrite == secremain)
            break; //д�������
        else       //д��δ����
        {
            secpos++;                //������ַ��1
            secoff = 0;              //ƫ��λ��Ϊ0
            pBuffer += secremain;    //ָ��ƫ��
            WriteAddr += secremain;  //д��ַƫ��
            NumToWrite -= secremain; //�ֽ�(16λ)���ݼ�
            if (NumToWrite > (STM_SECTOR_SIZE / 2))
                secremain = STM_SECTOR_SIZE / 2; //��һ����������д����
            else
                secremain = NumToWrite; //��һ����������д����
        }
    };
    HAL_FLASH_Lock(); //����
}
#endif

/**
  * ��������: ��ָ����ַ��ʼ����ָ�����ȵ�����
  * �������: ReadAddr:��ʼ��ַ
  *           pBuffer:����ָ��
  *           NumToRead:����(16λ)��
  * �� �� ֵ: ��
  * ˵    ������
  */
void STMFLASH_Read(uint32_t ReadAddr, uint16_t *pBuffer, uint16_t NumToRead)
{
    uint16_t i;

    for (i = 0; i < NumToRead; i++)
    {
        pBuffer[i] = STMFLASH_ReadHalfWord(ReadAddr); //��ȡ2���ֽ�.
        ReadAddr += 2;                                //ƫ��2���ֽ�.
    }
}

//------------------����Ϊ����ĥ��--------------------//
/*
    |0x00|0x00|
    |0x00|0x00|  // ���ÿռ�

    |0x5A|data|
    |0x5A|data|  // ��Ч����
    
    |0xff|0xff|  // δ�ÿռ�
    |0xff|0xff|
    |0xff|0xff|
*/
// ��δ�õĿռ䴦д��������
uint8_t flash_write_new(uint8_t *pBuffer, uint16_t NumToWrite)
{
    uint32_t addr = FLASH_START_ADDRESS;
    // uint32_t addr_used = FLASH_START_ADDRESS;
    uint16_t write_buf[STM_SECTOR_SIZE / 2] = {0};
    int i;
    uint32_t SECTORError = 0;

    // Ҫд�����ݶ���ҳ��С
    if (NumToWrite > STM_SECTOR_SIZE / 2)
    {
        return 0;
    }

    // // �ҵ���һ�����õİ���
    // while (addr_used < FLASH_END_ADDRESS && STMFLASH_ReadHalfWord(addr_used) == 0)
    // {
    //     addr_used += 2;
    // }

    // �ҵ���һ��δ�õİ���
    while (addr < FLASH_END_ADDRESS && STMFLASH_ReadHalfWord(addr) != 0xffff)
    {
        addr += 2;
    }

    // ���û�п��õĿռ� �� ʣ����õĿռ䲻�㹻�� -> ����ҳ
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

    // // ����ϴε���Ч����
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

// ���NumToClear����Ч����
uint8_t flash_clear_used(uint16_t NumToClear)
{
    uint32_t addr = FLASH_START_ADDRESS;
    uint16_t write_buf[STM_SECTOR_SIZE / 2] = {0};

    // �ҵ���һ��δ�õİ���
    while (addr < FLASH_END_ADDRESS && STMFLASH_ReadHalfWord(addr) != 0xffff)
    {
        addr += 2;
    }

    // ���û�ҵ�  ˵���պ�����
    //(�ƺ���Ҫ���ifҲ��)
    if (addr >= FLASH_END_ADDRESS)
    {
        addr = FLASH_END_ADDRESS;
    }

    // ���õĿռ䶼û��NumToClear��
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

// ���������Ч����
uint8_t flash_clear_all_used()
{
    uint32_t addr = FLASH_START_ADDRESS;
    uint16_t write_buf[STM_SECTOR_SIZE / 2] = {0};
    uint16_t NumToClear;

    // �ҵ���һ��δ�õİ���
    while (addr < FLASH_END_ADDRESS && STMFLASH_ReadHalfWord(addr) != 0xffff)
    {
        addr += 2;
    }

    // ���û�ҵ�  ˵���պ�����
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

// ������
// �����Ч���ݲ���NumToRead��������0
uint8_t flash_read(uint8_t *pBuffer, uint16_t NumToRead)
{
    uint32_t addr = FLASH_START_ADDRESS;
    uint16_t read_buf[STM_SECTOR_SIZE / 2];
    int i;

    // Ҫд�����ݶ���ҳ��С
    if (NumToRead > STM_SECTOR_SIZE / 2)
    {
        return 0;
    }

    // �ҵ���һ��δ�õİ���
    while (addr < FLASH_END_ADDRESS && STMFLASH_ReadHalfWord(addr) != 0xffff)
    {
        addr += 2;
    }

    // ���û�ҵ�  ˵���պ�����
    if (addr >= FLASH_END_ADDRESS)
    {
        addr = FLASH_END_ADDRESS;
    }

    // ���õĿռ䶼û��NumToWrite��
    if ((addr - FLASH_START_ADDRESS) / 2 < NumToRead)
    {
        return 0;
    }

    addr -= NumToRead * 2;

    // �ϴ�δд��NumToWrite��
    if ((STMFLASH_ReadHalfWord(addr) >> 8) != 0x5A)
    {
        return 0;
    }

    STMFLASH_Read(addr, read_buf, NumToRead);

    // �����־λ0x5a
    for (i = 0; i < NumToRead; i++)
    {
        pBuffer[i] = read_buf[i] % 1024;
    }

    return 1;
}

//////////////����///////////////////
// uint8_t flash_write_used(uint8_t *pBuffer, uint16_t NumToWrite)
// {
//     uint32_t addr = FLASH_START_ADDRESS;
//     uint16_t write_buf[STM_SECTOR_SIZE / 2];
//     int i;

//     // Ҫд�����ݶ���ҳ��С
//     if (NumToWrite > STM_SECTOR_SIZE / 2)
//     {
//         return 0;
//     }

//     // �ҵ���һ��δ�õİ���
//     while (addr < FLASH_END_ADDRESS && STMFLASH_ReadHalfWord(addr) != 0xffff)
//     {
//         addr += 2;
//     }

//     // ���û�ҵ�  ˵���պ�����
//     if (addr >= FLASH_END_ADDRESS)
//     {
//         addr = FLASH_END_ADDRESS;
//     }

//     // ���õĿռ䶼û��NumToWrite��
//     if ((addr - FLASH_START_ADDRESS) / 2 < NumToWrite)
//     {
//         return 0;
//     }

//     addr -= NumToWrite * 2;

//     // �ϴ�δд��NumToWrite��
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
/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/
