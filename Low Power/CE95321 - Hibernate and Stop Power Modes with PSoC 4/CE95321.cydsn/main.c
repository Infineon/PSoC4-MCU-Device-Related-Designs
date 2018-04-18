/******************************************************************************
* File Name: main.c
*
* Version: 1.00
*
* Description:
*   This example project demonstrates Stop and hibernate low power mode.
*   This project uses Low Power Comparator, Digital Pins, and
*   UART Component. It also shows SRAM retention in hibernate mode.
*
*******************************************************************************
* Copyright 2013, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license
* agreement accompanying the software package from which you obtained
* this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
******************************************************************************/

#include <device.h>

/* Macro definitions */
#define CONVERT_TO_ASCII		(48u)
#define HIGH				 	(1u)
#define LOW					 	(0u)
#define	LP_COMP_INTR_MASK		(0x03)
#define CLEAR_SCREEN			(12u)

/* Interrupt prototypes */
CY_ISR_PROTO(WakeupPin_ISR_Handler);
CY_ISR_PROTO(Comparator_ISR_Handler);

/* Initialize the peripherals as per the reset source */
void Initialize(void);

/* Convert the count value to ASCII and send it to UART */
void Convert_And_Send(const char8 []);

/* Execute the command received from UART and send the count value back */
void Execute_Command(char8);

/* Enter low power if switch press detected in low power mode */
void Check_SwitchPress(void);

/* Attribute CY_NOINIT puts SRAM variable in memory section which is retained in low power modes */
CY_NOINIT uint8 Count;
CY_NOINIT uint8 Count_ASCII;

/* Flag to detect switch press in active mode and enter low power mode */
uint32 LowPower_Flag;


/******************************************************************************
* Function Name: main
*******************************************************************************
*
* Summary:
*  main() performs following functions:
*  1: Initialize interrupts, peripherals and variables
*  2: Read the command from UART
*  3: Execute the command - Hibernate, Stop, Increment or Decrement depending
*     upon the received character
*  4: Enter low power mode from active mode by detecting the switch press
*     as below:
*     Pressing once enters Hibernate mode and Press and hold enters Stop mode
*
* Parameters:
*  None.
*
* Return:
*  None.
*
******************************************************************************/
void main()
{
    /* Data received from the serial port */
    char8 Input_Char = 0u;

    /* Initialize the peripherals as per the reset source */
    Initialize();

    /* Display the current count value through UART */
    Convert_And_Send(" Count Value ");

    while(1)
    {
        /* Check the UART status */
        Input_Char = UART_UartGetChar();

        /* If byte received, execute the command*/
        if (Input_Char > 0u)
        {
            /* Execute the command received from UART and send the count value back */
            Execute_Command(Input_Char);
        }

        /* If Switch press detected in active mode, enter low power mode
         * a. Device enters hibernate mode if pressed once
         * b. Device enter stop mode if switch if pressed and held
         */
        Check_SwitchPress();
    }
}


/******************************************************************************
* Function Name: Initialize
*******************************************************************************
*
* Summary:
*  Initialize() performs following functions:
*  1: Initialize interrupts
*  2: Check the source of reset and Initialize peripherals and variables
*     accordingly.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
******************************************************************************/
void Initialize(void)
{
    /* Start and clear the interrupts */
    isr_WakeupPin_StartEx(WakeupPin_ISR_Handler);
    isr_Comparator_StartEx(Comparator_ISR_Handler);

    /* Wait till the switch is pressed */
    while (Wakeup_Pin_Read() == 0u);
    CyDelay(100u);

    Wakeup_Pin_ClearInterrupt();
    isr_WakeupPin_ClearPending();

    LPComp_ClearInterrupt(LP_COMP_INTR_MASK);
    isr_Comparator_ClearPending();

    /* Enable all interrupts */
    CyGlobalIntEnable;

    /* Check the source of reset
     * 1. Wake Up from Hibernate - Restart Low Power Comparator and UART
     * 2. Any other reset - Initialize the Peripherals and SRAM variables
     */
    if (CySysPmGetResetReason() == CY_PM_RESET_REASON_WAKEUP_HIB)
    {
        /* Initialize variables */
        LowPower_Flag = 0u;

        /* Start Low Power Comparator */
        LPComp_Start();

        /* Start UART Component */
        UART_Start();
    }
    else
    {
        /* Initialize variables */
        Count = 0u;
        LowPower_Flag = 0u;

        /* Convert it to ASCII character */
        Count_ASCII = Count + CONVERT_TO_ASCII;

        /* Unfreeze GPIOs */
        CySysPmUnfreezeIo();

        /* Start Low Power Comparator */
        LPComp_Start();

        /* Start UART Component */
        UART_Start();
    }

    /* Turn the LED connected to Pin_LowPowerOut ON to indicate active mode */
    Pin_LowPowerOut_Write(LOW);

    /* Note: If LED is active HIGH, then replace "LOW" with "HIGH" */

    /* Set this Pin low to indicate the device is not in Stop mode */
    Pin_Stop_Write(LOW);
}


/******************************************************************************
* Function Name: Convert_And_Send
*******************************************************************************
*
* Summary:
*  Convert_And_Send() performs following functions:
*  1: Convert the present count value to ASCII character
*  2: Clear terminal screen
*  2: Write the corresponding string to UART and send the present count value
*
* Parameters:
*  const char8 Str[]:  Write the string on UART
*
* Return:
*  None.
*
******************************************************************************/
void Convert_And_Send(const char8 Str[])
{
    /* Convert the present count value to ascii */
    Count_ASCII = Count % 10u;
    Count_ASCII += CONVERT_TO_ASCII;

    /* Clear Screen */
    UART_UartPutChar(CLEAR_SCREEN);

    /* Write to UART */
    UART_UartPutString(Str);
    UART_UartPutChar(Count_ASCII);
}


/******************************************************************************
* Function Name: Execute_Command
*******************************************************************************
*
* Summary:
*  Execute_Command() performs following functions:
*  1: Reads the command from UART
*  2: Executes the corresponding function
*
* Parameters:
*  char8 Char_In:  Input character received from UART
*
* Return:
*  None.
*
******************************************************************************/
void Execute_Command(char8 Char_In)
{
    /* If 'S' is received through UART, set the device to enter stop mode */
    if((Char_In == 'S') || (Char_In == 's'))
    {
        /* Convert count value to ASCII to send it on UART */
        Convert_And_Send(" Stop Mode - Count Value ");
        CyDelay(10u);

        /* Turn the LED connected to Pin_LowPowerOut OFF to indicate low power mode */
        Pin_LowPowerOut_Write(HIGH);

        /* Note: If LED is active HIGH, then replace "HIGH" with "LOW" */

        /* Set the indicator Pin_Stop to HIGH to indicate stop Mode */
        Pin_Stop_Write(HIGH);

        /* Enter Stop Mode */
        CySysPmStop();
    }

    /* If 'H' is received through UART, set the device to enter hibernate mode */
    else if((Char_In == 'H') || (Char_In == 'h'))
    {
        /*Convert count value to ASCII to send it on UART */
        Convert_And_Send(" Hibernate Mode - Count Value ");
        CyDelay(10u);

        /* Turn the LED connected to Pin_LowPowerOut OFF to indicate low power mode */
        Pin_LowPowerOut_Write(HIGH);

        /* Note: If LED is active HIGH, then replace "HIGH" with "LOW" */

        /* Enter Hibernate Mode */
        CySysPmHibernate();
    }

    /* If 'I' is received through UART, increment the count value */
    else if((Char_In == 'I')||(Char_In == 'i'))
    {
        /* Increment count value and convert it to ASCII to send it on UART */
        Count++;
        if(Count == 0u)
        {
        	Count = 6u;
        }
        Convert_And_Send(" Increment Value ");
    }

    /* If 'D' is received through UART, decrement the count value */
    else if((Char_In == 'D')||(Char_In == 'd'))
    {
        /*Decrement count value and convert it to ASCII to send it on UART */
        Count--;
        if(Count == 255u)
        {
        	Count = 9u;
        }
        Convert_And_Send(" Decrement Value ");
    }

    /* If any other input is received, display UNKNOWN COMMAND - PRESS H S I D
     * on UART and transition on the Pin_LowPowerOut
     */
    else
    {
        /* Indicate Wrong command by showing transition in Pin_LowPowerOut */
        Pin_LowPowerOut_Write(HIGH);

        /* Note: If LED is active HIGH, then replace "HIGH" with "LOW" */

        CyDelay(100u);

        Pin_LowPowerOut_Write(LOW);

        /* Note: If LED is active HIGH, then replace "LOW" with "HIGH" */
        UART_UartPutChar(CLEAR_SCREEN);
        UART_UartPutString(" UNKNOWN COMMAND - PRESS H, S, I or D");
    }
}


/******************************************************************************
* Function Name: Check_SwitchPress
*******************************************************************************
*
* Summary:
*  Check_SwitchPress() performs following functions:
*  1: Checks the LowPower_Flag
*  2: Enters low power mode according to the flag
*  3: Converts and sends the present count value
*
* Parameters:
*  None.
*
* Return:
*  None.
*
******************************************************************************/
void Check_SwitchPress(void)
{
    /* Checks the LowPower_Flag
     * a. 1 - Enter Stop mode
     * b. 2 - Enter Hibernate mode
     * This flag is set in isr_WakeupPin.c
     */
    if (LowPower_Flag == 1u)
    {
        LowPower_Flag = 0u;

        /* Turn the LED connected to Pin_LowPowerOut OFF to indicate low power mode */
        Pin_LowPowerOut_Write(HIGH);

        /* Note: If LED is active HIGH, then replace "HIGH" with "LOW" */

        /* Convert and send the mode and present count value */
        Convert_And_Send(" Stop Mode - Count Value ");

        /* Wait till the switch is pressed */
        while (Wakeup_Pin_Read() == 0u);

        /* Set the Pin_Stop HIGH to indicate stop mode */
        Pin_Stop_Write(HIGH);

        CyDelay(10u);

        /* Enter stop mode*/
        CySysPmStop();
    }

    if (LowPower_Flag == 2u)
    {
        LowPower_Flag = 0u;

        /* Turn the LED connected to Pin_LowPowerOut OFF to indicate low power mode */
        Pin_LowPowerOut_Write(HIGH);

        /* Note: If LED is active HIGH, then replace "HIGH" with "LOW" */

        /* Convert and send the mode and present count value */
        Convert_And_Send(" Hibernate Mode - Count Value ");

        /* Wait till the switch is pressed */
        while (Wakeup_Pin_Read() == 0u);

        CyDelay(10u);

        /* Enter hibernate mode*/
        CySysPmHibernate();
    }
}


/*******************************************************************************
* Function Name: WakeupPin_ISR_Handler
********************************************************************************
*
* Summary:
*   Interrupt Service Routine for Wakeup_Pin. Checks if the button
*   has been pressed for a long or short time and selects low power mode
*   accordingly (long press for Stop mode).
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(WakeupPin_ISR_Handler)
{
    uint32 counter;

    /* Debounce delay */
    CyDelay(50u);

    /* If the switch is pressed for more than 1 sec, set the flag to enter Stop
     * mode else set the flag to enter hibernate mode. This flag will be cleared
     * by the device when the device wakes up from low power mode
     */
    for(counter = 0u; (counter < 20u) && (Wakeup_Pin_Read() == 0u); counter++)
    {
        CyDelay(50u);
    }

    LowPower_Flag = (counter == 20u) ? 1u : 2u;

	Wakeup_Pin_ClearInterrupt();
    isr_WakeupPin_ClearPending();
}


/*******************************************************************************
* Function Name: Comparator_ISR_Handler
********************************************************************************
*
* Summary:
*   The Interrupt Service Routine for LPComp.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(Comparator_ISR_Handler)
{
    isr_Comparator_ClearPending();
    if( (CY_GET_REG32(CYREG_LPCOMP_INTR) & 0x03u) != 0u )
    {
        CyDelayUs(100u);
        LPComp_ClearInterrupt(0x03u);
    }
}


/* [] END OF FILE */
