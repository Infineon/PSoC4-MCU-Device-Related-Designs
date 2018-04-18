/*******************************************************************************
* File: main.c
*
* Version: 1.0
*
* Description:
*  This is source code for example project that demonstrates basic
*  functionality of PSoC4 Watchdog.
*
********************************************************************************
* Copyright 2013-2014, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/
#include <project.h>

/* WDT counter configuration */
#define WDT_COUNT0_MATCH    (0x4FFFu)
#define WDT_COUNT1_MATCH    (0x0008u)

/* Prototype of WDT ISR */
CY_ISR_PROTO(WdtIsrHandler);


/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Determines the reset cause and blinks associated LED. Configures the WDT 
*  counters 0 and 1 to generate interrupts and reset the device respectively.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
int main()
{   
    /* Determine reset cause. */
    if (0u == CySysGetResetReason(CY_SYS_RESET_WDT))
    {
        /* Toggle LED_Reset at startup after PowerUp/XRES event. */
        LED_Reset_Write(0u);
        CyDelay(500u);
        LED_Reset_Write(1u);
    }
    else
    {
        /* Toggle LED_WdtReset at startup after WDT reset event. */
        LED_WdtReset_Write(0u);
        CyDelay(500u);
        LED_WdtReset_Write(1u);
    }

	/* Setup ISR for interrupts at WDT counter 0 events. */
    WdtIsr_StartEx(WdtIsrHandler);

    /* Enable global interrupts. */
    CyGlobalIntEnable;
	
	/* Set WDT counter 0 to generate interrupt on match */
	CySysWdtWriteMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_INT);
	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER0, WDT_COUNT0_MATCH);
	CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER0, 1u);
	
	/* Enable WDT counters 0 and 1 cascade */
	CySysWdtWriteCascade(CY_SYS_WDT_CASCADE_01);
    
	/* Set WDT counter 1 to generate reset on match */
	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER1, WDT_COUNT1_MATCH);
	CySysWdtWriteMode(CY_SYS_WDT_COUNTER1, CY_SYS_WDT_MODE_RESET);
    CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER1, 1u);
	
	/* Enable WDT counters 0 and 1 */
	CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK | CY_SYS_WDT_COUNTER1_MASK);
	
	/* Lock WDT registers and try to disable WDT counters 0 and 1 */
	CySysWdtLock();
	CySysWdtDisable(CY_SYS_WDT_COUNTER1_MASK);
	CySysWdtUnlock();
	
	for(;;)
    {

    }
}


/*******************************************************************************
* Function Name: WdtIsrHandler
********************************************************************************
* Summary:
*  Interrupt handler for WDT counter 0 interrupts. Toggles the LED_WdtInt pin.
* 
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
CY_ISR(WdtIsrHandler)
{
	/* Toggle pin state */
	LED_WdtInt_Write(~(LED_WdtInt_Read()));

    /* Clear interrupts state */
	CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER0_INT);
    WdtIsr_ClearPending();
}


/* [] END OF FILE */
