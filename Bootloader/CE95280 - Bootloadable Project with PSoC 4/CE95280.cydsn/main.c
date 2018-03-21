/*******************************************************************************
* File: main.c
*
* Version: 1.0
*
* Description:
*  This example project demonstrates the basic operation of Bootloadable
*  component.
*
********************************************************************************
* Copyright 2014, Cypress Semiconductor Corporation. All rights reserved.
* This software is owned by Cypress Semiconductor Corporation and is protected
* by and subject to worldwide patent and copyright laws and treaties.
* Therefore, you may use this software only as provided in the license agreement
* accompanying the software package from which you obtained this software.
* CYPRESS AND ITS SUPPLIERS MAKE NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* WITH REGARD TO THIS SOFTWARE, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT,
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*******************************************************************************/
#include <project.h>

int main()
{
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */

    CyGlobalIntEnable;

    /* Turn on green LED */
    Bootloadable_Status_Write(0u);

    /* Make a delay */
    CyDelay(7000u);

    /* Schedule bootloader application and execute software reseet */
    Bootloadable_Load();

    for(;;)
    {
        /* Should never be here */
    }
}

/* [] END OF FILE */
