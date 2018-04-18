/*******************************************************************************
*
* Filename:             main.c
* Owner :               Bob Hu (bobh@cypress.com)
*
* Version:              V1.0 
* Description:          This file demonstrates how to use Watchdog to reset CY8C41/42
*                       devices when program is out of control and how to use 
*                       Watchdog to wake CY8C41/42 devices from deep sleep mode
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
* Code Tested with:     PSoC Creator  3.1 SP2 (3.0.0.3140);
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
 * These Macro are only used in this module for button pressing event 
 * detection. These Macro should not be populated to other modules.
 ******************************************************************************/
#define LED_ON                      (uint8)0x00
#define LED_OFF                     (uint8)0x01
#define WDT_INTERVAL_1S             1000u                       /* millisecond */
#define SLEEP_INTERVAL_250MS        250u                        /* millisecond */
#define WAKE_TIME                   0x0FFF                      /* loop count */
#define ILO_FREQ                    32000                       /* Hz */
#define LOG_ROW_INDEX               (CY_FLASH_NUMBER_ROWS - 1)  /* last row */
#define DoSomething(void)                                       /* just delay 500ms */

/******************************************************************************
 * Global variables definition
 * ----------------------------------------------------------------------------
 * These varialbes should be populated to other modules. Header file contain 
 * the extern statement for these variables.
 ******************************************************************************/ 
/* counting the time that entering the Watchdog interrupt ISR routine */
volatile uint8 wdtIsrCount = 0;


void InitWatchdog(uint16 sleep_interval, uint16 reset_internal);
CY_ISR(isr_WDT);

int main()
{
    uint8 resetCause = 0;    
    uint16 wakeCounter = WAKE_TIME;

    /*===========================================================================================
     * this code piece detects the reset cause, if the last reset is caused by watchdog, a red LED
     * indicator is turn on
     *==========================================================================================*/
    /* Get reset cause after system powered */
    resetCause = CySysGetResetReason(CY_SYS_RESET_WDT | CY_SYS_RESET_SW | CY_SYS_RESET_PROTFAULT);
    if(resetCause == CY_SYS_RESET_WDT)
    {        
        /* turn on Red LED to indicate system id reset by watchdog */
        LED_Green_Write(LED_OFF);
        LED_Red_Write(LED_ON);
        LED_Blue_Write(LED_OFF);
        
        /* Delay 1 second */
        CyDelay(1000);
    }

    /*===========================================================================================
     * this code piece turns on Green LED when normally system working 
     *==========================================================================================*/
    /* turn on Green LED to indicate system works normally */
    LED_Green_Write(LED_ON);
    LED_Red_Write(LED_OFF);
    LED_Blue_Write(LED_OFF);
    
    /* initiate button scan module */
    ButtonInit();
    
    /*===========================================================================================
     * this code piece initializes the watchdog function 
     *==========================================================================================*/
    /* reset watchdog ISR routine counter */
    wdtIsrCount = 0;
    /* initialize watchdog */
    InitWatchdog(SLEEP_INTERVAL_250MS, WDT_INTERVAL_1S);
    /* connect ISR routine to Watchdog interrupt */
    ISR_WDT_StartEx(isr_WDT);
    /* set the highest priority to make ISR is executed in all condition */
    ISR_WDT_SetPriority(0);
    /* enable global interrupt */
    CyGlobalIntEnable; 
        
    for(;;)
    {
        /* ===============================================================
         * do something here for your system 
         * ===============================================================*/
        DoSomething();

        ButtonProcess();
        if(btnStatus[0] == BUTTON_ON)
        {
            /* turn on blue LED to indicate button is pressed*/
            LED_Green_Write(LED_OFF);
            LED_Red_Write(LED_OFF);
            LED_Blue_Write(LED_ON);

            /* ===============================================================*/
            /* stop feeding the watchdog, if keeping SW2 is pressed, 
               the system is reset by Watchdog after 3 seconds */
        }
        else
        {   
            /* this wakeCounter is used to control the turn-on time for RED led */
            if(wakeCounter == 0)
            {
                /* turn on Green LED to indicate system works normally */
                LED_Green_Write(LED_ON);
                LED_Red_Write(LED_OFF);
                LED_Blue_Write(LED_OFF);
            
                /* clear watchdog counter before deep sleep */
                CySysWdtResetCounters(CY_SYS_WDT_COUNTER0_RESET);
                /* reset watchdog counter requires several LFCLK cycles to take effect */
                CyDelayUs(150);            
                /* go to deep sleep mode */
                CySysPmDeepSleep();
                
                /* reset wakeCounter value */
                wakeCounter = WAKE_TIME;
                /* turn on Red LED after wakeup by watchdog interrupt */
                LED_Green_Write(LED_OFF);
                LED_Blue_Write(LED_OFF);
                LED_Red_Write(LED_ON); 
            }
            else
            {
                wakeCounter--;
            }
            
            /* ===============================================================*/
            /* clear watchdog counter after your task completes */
            CySysWdtResetCounters(CY_SYS_WDT_COUNTER1_RESET);  
            /* reset watchdog counter requires several LFCLK cycles to take effect */
            CyDelayUs(150);            
            /* reset watchdog ISR counter after succeeding in feeding watchdog */
            wdtIsrCount = 0;
        }
    }
}

/*******************************************************************************
* Function Name: InitWatchdog
********************************************************************************
* Summary:
*   Initialize watchdog counter0 with specific sleep interval parameter
*   Initialize watchdog counter1 with specific reset interval parameter
*
* Parameters:  
*   uint16 sleep_interval   sleep interval, in millisecond unit
*   uint16 reset_interval   reset interval, in millisecond unit
*
* Return: 
*   void
*
*******************************************************************************/
void InitWatchdog(uint16 sleep_interval, uint16 reset_internal)
{
    /*==============================================================================*/
    /* configure counter 0 for wakeup interrupt                                     */
    /*==============================================================================*/
    /* Counter0 of Watchdog time generates peridically interrupt to wakeup system */
    CySysWdtWriteMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_INT);
    /* Set interval as desired value */
	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER0, (sleep_interval * ILO_FREQ / 1000));
    /* clear counter on match event */
	CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER0, 1u);
    
    /*==============================================================================*/
    /* configure counter 1 for system reset                                         */
    /*==============================================================================*/
    /* Counter1 of Watchdog time generates peridically interrupt and a 
       reset on the third unhandled interrupt */
    CySysWdtWriteMode(CY_SYS_WDT_COUNTER1, CY_SYS_WDT_MODE_INT_RESET);
    /* Set interval as desired value */
	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER1, (reset_internal / sleep_interval));
    /* clear counter on match event */
	CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER1, 1u);
    
    /*==============================================================================*/
    /* cascade them and enable watchdog                                             */
    /*==============================================================================*/
    /* cascade Counter 0 and Counter 1 */
    CySysWdtWriteCascade(CY_SYS_WDT_CASCADE_01);    
    /* enable the counter 0 */
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK | CY_SYS_WDT_COUNTER1_MASK);    
    /* check if counter 0 is enabled, otherwise keep looping here */
    while(!CySysWdtReadEnabledStatus(CY_SYS_WDT_COUNTER0));
    /* check if counter 1 is enabled, otherwise keep looping here */
    while(!CySysWdtReadEnabledStatus(CY_SYS_WDT_COUNTER1));
}

/*******************************************************************************
* Function Name: isr_WDT
********************************************************************************
* Summary:
*   interrupt service routine to handle watchdog interrupt, clear interrupt flag
*   for system wakeup and clear interrupt flag for system reset in normal working. 
*   Store fault data into Flash if program is out of control
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
    uint32 intSource = CySysWdtGetInterruptSource();
    
    /* if interrupt is generated by COUNTER 0 */
    if((intSource & CY_SYS_WDT_COUNTER0_INT) == CY_SYS_WDT_COUNTER0_INT)
    {
        CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER0_INT);
    }
    /* if interrupt is generated by COUNTER 1 */
    if((intSource & CY_SYS_WDT_COUNTER1_INT) == CY_SYS_WDT_COUNTER1_INT)
    {
        if(wdtIsrCount == 0)
        {
            /* normally feeding the watchdog and clear interrupt flag */
            CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER1_INT);
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
                /* succeed in writting flash, stop the ISR response for following */
                ISR_WDT_Stop();
            }
            /* do not clear interrupt flag for Watchdog interrupt. The system reset
               is generated at the third interrrupt */
        }
        /* increase this counter to indentify if clearing watchdog works well */
        wdtIsrCount++;
    }
}

/* [] END OF FILE */
