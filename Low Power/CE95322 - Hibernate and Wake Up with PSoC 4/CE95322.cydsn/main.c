/*******************************************************************************
* File: main.c
*
* Version: 1.0
*
* Description:
*  This is source code for the example of the Power Management API, which is
*  part of the cy_boot component. This project demonstrates the Hibernate low
*  power mode usage for PSoC 4 devices.
*
********************************************************************************
* Copyright 2013, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/
#include <device.h>
#include <cyPm.h>

CY_NOINIT uint16 cyHibernatesCount;


/*******************************************************************************
* Function Name: WakeupIsr
********************************************************************************
*
* Summary:
*  The Interrupt Service Routine for wakeup event.
*
* Parameters:  
*  None
*
* Return:
*  None
*
*******************************************************************************/
CY_ISR(isr_wakeup)
{
    pin_0_1_wakeup_isr_ClearInterrupt();
    CyDelay(100);
}
 

/*******************************************************************************
* Function Name: main
********************************************************************************
*
* Summary:
*  Main function performs following functions:
*   1. Turns LED1 on
*   2. Starts Character LCD component and displays project's information
*   3. Disaplay last reset reason
*   4. Enable global interrupts
*   5. The following steps are taken in the infinite loop:
*       5.1. The delay is made for visibility.
*       5.2. Indicate that IO-Cells will be frozen (blinking asterisks)
*       5.3. Prepare Character LCD component for the Hibernate mode.
*       5.4. Turn off LED1 to indicate Hibernate mode.
*       5.5. Switch to the Hibernate Mode.
*       5.6. Wake up from Hibernate mode is performed by SW1 button.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void main()
{
    uint32 reason;

    /* Unfreeze IO-Cells */
    CySysPmUnfreezeIo();

    /* Indicate an active mode */
    pin_0_0_toggle_Write(1);

    /* Setup ISR */
    CyIntSetVector(0, isr_wakeup);
    CyIntEnable(0);

    LCD_Start();
    LCD_ClearDisplay();

    LCD_PrintString("PM API Example");

    LCD_Position(1, 0);
    
    /* Retrieve and print last reset reason */
    reason = CySysPmGetResetReason();
    switch (reason)
    {
        case CY_PM_RESET_REASON_WAKEUP_STOP:
            cyHibernatesCount = 0;
            LCD_PrintString("Stop");
            break;
        case CY_PM_RESET_REASON_WAKEUP_HIB:
            ++cyHibernatesCount;
            LCD_PrintString("Hibernate");
            LCD_Position(1, 10);
            LCD_PrintNumber(cyHibernatesCount);
            break;
        case CY_PM_RESET_REASON_XRES:
            cyHibernatesCount = 0;
            LCD_PrintString("XRES");
            break;
        default:
            cyHibernatesCount = 0;
            LCD_PrintString("Unknown");
    }

    /* Enabling global interrupts. */
    CyGlobalIntEnable;

    while(1)
    {
        CyDelay(2000);

        /* Indicate that IO-Cells will be frozen */
        LCD_Position(1, 13);
        LCD_PrintString("***");
        CyDelay(300);
        LCD_Position(1, 13);
        LCD_PrintString("   ");
        CyDelay(300);
        LCD_Position(1, 13);
        LCD_PrintString("***");
        CyDelay(300);
        LCD_Position(1, 13);
        LCD_PrintString("   ");
        CyDelay(300);

        /* Indicate enter to the Sleep mode */
        pin_0_0_toggle_Write(0);

        /* Prepare Character LCD component for the Sleep mode */
        LCD_Sleep();

        /* Freeze IO-Cells */
        CySysPmFreezeIo();

        CySysPmHibernate();
        pin_0_0_toggle_Write(1);
    }
}


/* [] END OF FILE */
