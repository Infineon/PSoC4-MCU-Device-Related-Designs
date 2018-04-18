/*******************************************************************************
* File Name: main.c  
* Version 1.0
*
* Description:
*  Contains the main.c function.
*
*
*******************************************************************************
* Copyright 2013, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
********************************************************************************/

#include <device.h>


CY_ISR(WT_ISR)
{
    static uint8 toggleVal;
    
    /* Clear the WDT interrupt */
    CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER0_INT);
        
    toggleVal ^= 1;
    Pin_WT_Write(toggleVal);
}

void main()
{	
    uint16 clocksComp, clocks;
    uint16 lastMatch;
    
    clocks = 64;
    
    isr_1_StartEx(WT_ISR);
	
    /* Configure for interrupt mode for WDT 0 */
    CySysWdtWriteMode(0, CY_SYS_WDT_MODE_INT);
    /* Let the timer clear when it reaches period */
    CySysWdtWriteClearOnMatch(0, 1);
    /* Set the period for the WDT */
    CySysWdtWriteMatch(0,clocks - 1);
    /* Enable the WDT */
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK);
    
    ILO_Trim_Start();
        
	/* ILO_Trim uses an internal interrupt */
	CyGlobalIntEnable;
	
	for(;;)
	{
        clocksComp = ILO_Trim_Compensate(clocks);
        CySysWdtWriteMatch(0,clocksComp - 1);
        lastMatch = CySysWdtReadCount(0);
        if (lastMatch > clocksComp - 1)
        {
            /* Counter value must be below the match limit */
            CySysWdtResetCounters(CY_SYS_WDT_COUNTER0_RESET);
        }

        CyDelay(1000);
	}  
}

/* [] END OF FILE */
