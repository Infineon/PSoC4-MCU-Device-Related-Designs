/*******************************************************************************
*
* Filename:             main.c
* Owner :               Bob Hu (bobh@cypress.com)
*
* Version:              V1.0 
* Description:          This file demonstrates how to use Watchdog interrupt to 
*                       wake CY8C41/42 devices from deep sleep mode
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

/******************************************************************************
 * Macro definition
 * ----------------------------------------------------------------------------
 * These Macro are only used in this module for button pressing event 
 * detection. These Macro should not be populated to other modules.
 ******************************************************************************/
#define LED_ON                      (uint8)0x00
#define LED_OFF                     (uint8)0x01
#define SLEEP_INTERVAL_250MS        250                       /* millisecond */
#define ILO_FREQ                    32000                       /* Hz */
#define LOG_ROW_INDEX               (CY_FLASH_NUMBER_ROWS - 1)  /* last row */
#define DoSomething(void)           CyDelay(500)                /* just delay 500ms */

/******************************************************************************
 * Global variables definition
 * ----------------------------------------------------------------------------
 * These varialbes should be populated to other modules. Header file contain 
 * the extern statement for these variables.
 ******************************************************************************/         
void InitWatchdog(uint16 sleep_interval);
CY_ISR(isr_WDT);

int main()
{
    /*===========================================================================================
     * this code piece turns on Green LED for normally system working 
     *==========================================================================================*/
    /* turn on Green LED to indicate system is powered up */
    LED_Green_Write(LED_ON);
    LED_Red_Write(LED_OFF);
    LED_Blue_Write(LED_OFF);    
    /* delay for a while to give enough time for Green LED display */
    CyDelay(1000);
    
    /*===========================================================================================
     * this code piece initializes the watchdog function 
     *==========================================================================================*/
    /* initialize watchdog */
    InitWatchdog(SLEEP_INTERVAL_250MS);
    /* connect ISR routine to Watchdog interrupt */
    ISR_WDT_StartEx(isr_WDT);
    /* set the highest priority to make ISR is executed in all condition */
    ISR_WDT_SetPriority(0);
    /* enable global interrupt */
    CyGlobalIntEnable; 

    /*===========================================================================================
     * the main turn on Blue LED and then enter deep sleep for SLEEP_INTERVAL_500MS, when system 
     * is waked up by Watchdog interrupt, the Red LED is turn on for a while. This flow is repeated
     *==========================================================================================*/
    for(;;)
    {
        /* turn on Green LED to indicate system is in active mode */
        LED_Green_Write(LED_ON);
        LED_Red_Write(LED_OFF);
        LED_Blue_Write(LED_OFF);   
        
        /* clear watchdog counter before deep sleep */
        CySysWdtResetCounters(CY_SYS_WDT_COUNTER0_RESET);
        /* reset watchdog counter requires several LFCLK cycles to take effect */
        CyDelayUs(150); 
        /* go to deep sleep mode */
        CySysPmDeepSleep();
        
        /* turn on Red LED after wakeup by Watchdog interrupt */
        LED_Green_Write(LED_OFF);
        LED_Blue_Write(LED_OFF);
        LED_Red_Write(LED_ON);        
        
        /* do something here after wakeup from deep sleep*/
        DoSomething();  
    }
}

/*******************************************************************************
* Function Name: InitWatchdog
********************************************************************************
* Summary:
*   Initialize watchdog counter0 with specific sleep interval parameter
*
* Parameters:  
*   uint16 sleep_interval   reset interval, in millisecond unit
*
* Return: 
*   void
*
*******************************************************************************/
void InitWatchdog(uint16 sleep_interval)
{
    /*==============================================================================*/
    /* configure counter 0 for wakeup interrupt                                     */
    /*==============================================================================*/
    /* Counter 0 of Watchdog time generates peridically interrupt to wakeup system */
    CySysWdtWriteMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_INT);
    /* Set interval as desired value */
	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER0, ((uint32)(sleep_interval * ILO_FREQ) / 1000));
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
    /* clear interrupt flag to enable next interrupt */
    CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER0_INT);     
}

/* [] END OF FILE */
