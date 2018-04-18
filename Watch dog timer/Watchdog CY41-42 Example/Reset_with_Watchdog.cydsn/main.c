/*******************************************************************************
*
* Filename:             main.c
* Owner :               Bob Hu (bobh@cypress.com)
*
* Version:              V1.0 
* Description:          This file demonstrates how to use Watchdog to reset 
*                       CY8C41/42 family devices            
*                       
* -------------------------------------------------------------------------------
* ChangeList: 
*   V1.0                Initial version
* -------------------------------------------------------------------------------
* Known issues:         
*   V1.0                N/A
* -------------------------------------------------------------------------------
* Hardare Dependency:   
*   1. CY8CKIT-042(MCU board)
* -------------------------------------------------------------------------------
* Related documents:
*   N/A
* -------------------------------------------------------------------------------
* Code Tested with:     PSoC Creator  3.1 (3.1.0.1570);
*                       ARM GCC 4.7.3;
*
********************************************************************************
* Copyright (2014), Cypress Semiconductor Corporation.
********************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is 
* protected by and subject to worldwide patent protection (United States and 
* foreign), United States copyright laws and international treaty provisions. 
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable 
* license to copy, use, modify, create derivative works of, and compile the 
* Cypress Source Code and derivative works for the sole purpose of creating 
* custom software in support of licensee product to be used only in conjunction 
* with a Cypress integrated circuit as specified in the applicable agreement. 
* Any reproduction, modification, translation, compilation, or representation of 
* this software except as specified above is prohibited without the express 
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH 
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the 
* materials described herein. Cypress does not assume any liability arising out 
* of the application or use of any product or circuit described herein. Cypress 
* does not authorize its products for use as critical components in life-support 
* systems where a malfunction or failure may reasonably be expected to result in 
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of 
* such use and in doing so indemnifies Cypress against all charges. Use may be 
* limited by and subject to the applicable Cypress software license agreement. 
*******************************************************************************/
#include <project.h>
#include "userinterface.h"

/******************************************************************************
 * Macro definition
 * ----------------------------------------------------------------------------
 * These Macros are only used in this module for button pressing event detection. 
 * These Macros should not be populated to other modules.
 ******************************************************************************/
#define LED_ON                      (uint8)0x00
#define LED_OFF                     (uint8)0x01
#define WDT_INTERVAL_1S             1000u                       /* millisecond */
#define ILO_FREQ                    32000                       /* Hz */
#define LOG_ROW_INDEX               (CY_FLASH_NUMBER_ROWS - 1)  /* last row */
#define DoSomething(void)                                       /* empty macro definition */

/******************************************************************************
 * Global variables definition
 * ----------------------------------------------------------------------------
 * These variables can be populated to other modules. Header file contains 
 * the extern statement for these variables.
 ******************************************************************************/ 
/* counting the amount for entering the Watchdog interrupt service routine (ISR) */
volatile uint8 wdtIsrCount = 0;
         
void InitWatchdog(uint16 reset_interval);
CY_ISR(isr_WDT);

int main()
{
    uint8 resetCause = 0;
    
    /*===========================================================================================
     * this code piece detects the reset cause, if the last reset is caused by watchdog, a red LED
     * indicator is turn on
     *==========================================================================================*/
    /* Get reset cause after system is powered */
    resetCause = CySysGetResetReason(CY_SYS_RESET_WDT | CY_SYS_RESET_SW | CY_SYS_RESET_PROTFAULT);
    if(resetCause == CY_SYS_RESET_WDT)
    {        
        /* turn on Red LED to indicate system is reset by watchdog */
        LED_Green_Write(LED_OFF);
        LED_Red_Write(LED_ON);
        LED_Blue_Write(LED_OFF);
        
        /* Delay one second */
        CyDelay(1000);
    }
    /*===========================================================================================
     * this code piece turns on the Green LED to indicate that system works normally 
     *==========================================================================================*/
    /* turn on Green LED to indicate system works normally */
    LED_Green_Write(LED_ON);
    LED_Red_Write(LED_OFF);
    LED_Blue_Write(LED_OFF);
    
    /* initiate button scanning module */
    ButtonInit();    
    
    /*===========================================================================================
     * this code piece initializes the watchdog function 
     *==========================================================================================*/
    /* reset watchdog ISR routine counter */
    wdtIsrCount = 0;
    /* initialize watchdog */
    InitWatchdog(WDT_INTERVAL_1S);
    /* connect ISR routine to Watchdog interrupt */
    ISR_WDT_StartEx(isr_WDT);
    /* set the highest priority to make ISR execute in all condition */
    ISR_WDT_SetPriority(0);
    /* enable global interrupt */
    CyGlobalIntEnable; 
    
    /*===========================================================================================
     * the main loop polls the SW2 status; if SW2 is kept pressed, stop feeding the watchdog and system
     * is reset after three unhandled interrupts (3 * WDT_INTERVAL_1S). For the first unhandled
     * interrupt, some log data is stored into the last row of Flash
     *==========================================================================================*/
    for(;;)
    {
        /* ===============================================================
         * do something here for your system 
         * ===============================================================*/
        DoSomething();
        
        /* scan the button status */
        ButtonProcess();
        if(btnStatus[0] == BUTTON_ON)
        {
            /* turn on the blue LED to indicate that button is pressed*/
            LED_Green_Write(LED_OFF);
            LED_Red_Write(LED_OFF);
            LED_Blue_Write(LED_ON);

            /* ===============================================================*/
            /* stop feeding the watchdog, if SW2 is held (kept pressed), 
               the system is reset by watchdog after 3 seconds */
        }
        else
        {   
            /* turn on the green LED to indicate that system works normally */
            LED_Green_Write(LED_ON);
            LED_Red_Write(LED_OFF);
            LED_Blue_Write(LED_OFF);

            /* ===============================================================*/
            /* clear watchdog counter after your task completes */
            CySysWdtResetCounters(CY_SYS_WDT_COUNTER0_RESET);  
            /* clearing watchdog counter requires several LFCLK cycles to take effect */
            CyDelayUs(150);
            
            /* reset watchdog ISR counter after feeding watchdog sucessfully */
            wdtIsrCount = 0;
        }
    }
}

/*******************************************************************************
* Function Name: InitWatchdog
********************************************************************************
* Summary:
*   Initialize watchdog counter 0 with specific reset interval
*
* Parameters:  
*   uint16 reset_interval   reset interval, in ms unit
*
* Return: 
*   void
*
*******************************************************************************/
void InitWatchdog(uint16 reset_interval)
{
    /*==============================================================================*/
    /* configure counter 0 for system reset                                         */
    /*==============================================================================*/
    /* Counter 0 of watchdog generates peridically interrupt and a 
       reset is generated on the the third unhandled interrupt */
    CySysWdtWriteMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_INT_RESET);
    /* Set interval as desired value */
	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER0, (((uint32)(reset_interval * ILO_FREQ))/1000));
    /* clear counter on match event */
	CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER0, 1u);
    
    /*==============================================================================*/
    /* enable watchdog                                                              */
    /*==============================================================================*/
    /* enable the counter 0 */
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK);
    /* check if counter 0 is enabled, otherwise keep looping here */
    while(!CySysWdtReadEnabledStatus(CY_SYS_WDT_COUNTER0));
}

/*******************************************************************************
* Function Name: isr_WDT
********************************************************************************
* Summary:
*   interrupt service routine to handle watchdog interrupt, clear interrupt flag
*   in normal working or store log data into flash when program is out of control
*
* Parameters:  
*   void
*
* Return: 
*   void
*
*******************************************************************************/
CY_ISR(isr_WDT)
{     
    if(wdtIsrCount == 0)
    {
        /* clear interrupt flag */
        CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER0_INT);   
    }
    else
    {        
        /* this code piece is executed in the first time to enter  after counter 0 
           is not reset. This means some error happens to prevent feeding the 
           watchdog, or the system is out of control. Some system information may
           need to store into flash for fault analysis */
        uint8 log[CYDEV_FLS_ROW_SIZE] = {0};
        uint8 i = 0;
        uint8 status = 0;
        
        /* construct a test log data for flash writting */
        for(i = 0; i < CYDEV_FLS_ROW_SIZE; i++)
            log[i] = i;
        /* note: Flash writting requires 20ms. During this time, the device 
           should not be reset, or unexpected changes may be made to portions 
           of the flash. Reset sources include the XRES pin, a software reset,
           and the watchdog. Make sure that these are not inadvertently activated. 
           Also, the low voltage detect circuits should be configured to generate
           an interrupt instead of a reset. Do not do other things. */     
        status = CySysFlashWriteRow(LOG_ROW_INDEX, log); 
        if(status == CYRET_SUCCESS)
        {
            /* succeed in writting flash, stop the ISR response for following interrupt */
            ISR_WDT_Stop();
        }
        /* do not clear interrupt flag for Watchdog interrupt. The system reset
           is generated at the third interrrupt */
    }
    /* increase this counter to indentify if clearing watchdog works well */
    wdtIsrCount++;
}

/* [] END OF FILE */
