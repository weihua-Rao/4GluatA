#include "main.h"
#include <stdio.h>
/**
 *  Flash_Write_Protection
 */

#define FLASH_PAGE_SIZE        ((uint16_t)0x800)
#define FLASH_WRITE_START_ADDR ((uint32_t)0x08010000)
#define FLASH_WRITE_END_ADDR   ((uint32_t)0x08012000)

#define FLASH_PROTECTED_PAGES FLASH_WRP_Pages32to33 /* FLASH_WRITE_START_ADDR ~ FLASH_WRITE_END_ADDR */

/* Uncomment this line to Enable Write Protection */
#define WRITE_PROTECTION_ENABLE
/* Uncomment this line to Disable Write Protection */
//#define WRITE_PROTECTION_DISABLE

/**
 * @brief  Main program.
 */
int tttt(void)
{
    uint32_t WRPR_Value = 0xFFFFFFFF, ProtectedPages = 0;

    /* Write Protection */

    /* Unlocks the FLASH Program Erase Controller */
    FLASH_Unlock();

    /* Get pages already write protection */
    WRPR_Value = FLASH_GetWriteProtectionOB();

#ifdef WRITE_PROTECTION_DISABLE

    printf("Write Protection Disable\r\n");

    /* Get pages already write protected */
    ProtectedPages = ~(WRPR_Value | FLASH_PROTECTED_PAGES);

    /* Check if desired pages are already write protected */
    if ((WRPR_Value | (~FLASH_PROTECTED_PAGES)) != 0xFFFFFFFF)
    {
        /* Erase all the option Bytes */
        FLASH_EraseOB();

        /* Check if there is write protected pages */
        if (ProtectedPages != 0x0)
        {
            /* Restore write protected pages */
            FLASH_EnWriteProtection(ProtectedPages);
        }
        /* Generate System Reset to load the new option byte values */
        NVIC_SystemReset();
    }
#elif defined WRITE_PROTECTION_ENABLE

    /* Set write protected pages */
    ProtectedPages = (~WRPR_Value) | FLASH_PROTECTED_PAGES;

    /* Check if desired pages are not yet write protected */
    if (((~WRPR_Value) & FLASH_PROTECTED_PAGES) != FLASH_PROTECTED_PAGES)
    {
        /* Erase all the option Bytes */
        FLASH_EraseOB();

        /* Enable the pages write protection */
        FLASH_EnWriteProtection(ProtectedPages);

        /* Generate System Reset to load the new option byte values */
        NVIC_SystemReset();
    }
//    else
//    {
//        /* FLASH Write Protection Test */
//        printf("Flash Page Erase/Program\r\n");
//        /* Clear All pending flags */
//        FLASH_ClearFlag(FLASH_STS_CLRFLAG);

//        /* Erase */
//        if (FLASH_ERR_WRP == FLASH_EraseOnePage(FLASH_WRITE_START_ADDR))
//        {
//            /* Clear All pending flags */
//            FLASH_ClearFlag(FLASH_STS_CLRFLAG);

//            /* Program */
//            if (FLASH_ERR_WRP == FLASH_ProgramWord(FLASH_WRITE_START_ADDR, ProgramData))
//            {
//                /* Check */
//                if (ProgramData == (*(__IO uint32_t*)FLASH_WRITE_START_ADDR))
//                {
//                    /* Test Fail */
//                    Test_Result = FAILED;
//                }
//                else
//                {
//                    /* Test PASSED */
//                    Test_Result = PASSED;
//                }
//            }
//            else
//            {
//                /* Test Fail */
//                Test_Result = FAILED;
//            }
//        }
//        else
//        {
//            /* Test Fail */
//            Test_Result = FAILED;
//        }
//    }

#endif
}


/**
 * @brief  Retargets the C library printf function to the USART1.
 * @param
 * @return
 */
int fputc(int ch, FILE* f)
{
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXDE) == RESET) ;
    return (ch);
}

/*  */
/**
 * @brief  Retargets the C library scanf function to the USART1.
 * @param
 * @return
 */
int fgetc(FILE* f)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXDNE) == RESET) ;
    return (int)USART_ReceiveData(USART1);
}

/**
 * @}
 */

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
*          line: assert_param error line source number
 * @return None
 */
void assert_failed(const uint8_t* expr, const uint8_t* file, uint32_t line)
{
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1)
    {}
}

/**
 * @}
 */
#endif