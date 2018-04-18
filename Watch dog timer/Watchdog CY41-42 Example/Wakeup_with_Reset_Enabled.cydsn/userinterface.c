/*******************************************************************************
*
* Filename:             interface.c
* Owner :               Bob Hu (bobh@cypress.com)
*
* Version:              V1.0 
* Description:          
*                       This file handles the event for buttons. This file is a 
*                       reused module that contains a software glich filter for 
*                       button pressing and releasing event. This file does NOT
*                       affect the watchdog usage.                       
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
 * Global variables definition
 * ----------------------------------------------------------------------------
 * These varialbes should be populated to other modules. Header file contain 
 * the extern statement for these variables.
 ******************************************************************************/ 

/* press status for button SW1, default is BUTTON_OFF */
uint8 btnStatus[BTN_COUNT];

/******************************************************************************
 * Local Macro definition
 * ----------------------------------------------------------------------------
 * These Macro are only used in this module for button pressing event 
 * detection. These Macro should not be populated to other modules.
 ******************************************************************************/
#define BTN_LOW_LEVEL                       (uint8)(0x00)
#define BTN_HIHG_LEVEL                      (uint8)(0x01)

/* this macro define the initial level when starting to detect button pressing */
#define BTN_OFF_LEVEL                       BTN_HIHG_LEVEL
/* this macro defines the desired level when button is pressed */
#define BTN_ON_LEVEL                        BTN_LOW_LEVEL

/* debounce count for button glitch filter */
#define BTN_GLITCH_FILTER_ACTIVE_CNT        (uint8)(15)
#define BTN_GLITCH_FILTER_DISCARD_CNT       (uint8)(5)
#define BTN_GLITCH_FILTER_INIT_CNT          (uint8)(10)

/******************************************************************************
 * Local variables definition
 * ----------------------------------------------------------------------------
 * These varialbes are only used in this module for button pressing event 
 * detection. These varialbes should not be populated to other modules.
 ******************************************************************************/ 
typedef struct _Btn_Status_T
{
    uint8   preBtnStatus;                   /* varialbe to store last status of button level  */
    uint8   glitchFilter;                   /* glitch filter counter for button pressing */
    uint8   btnIsDetectFlag;                 /* flag to store button pressing event */ 
}Btn_Status_T;

static Btn_Status_T btnArray[BTN_COUNT];

/*******************************************************************************
* Function Name: Button_Init
********************************************************************************
* Summary:
*   Initialize button status structure to detect pressing event
*
* Parameters:  
*   void
*
* Return: 
*   void
*
*******************************************************************************/
void ButtonInit(void)
{
    uint8 i = 0;         
    for(i = 0; i < BTN_COUNT; i++)
    {
        btnArray[i].preBtnStatus = BTN_OFF_LEVEL;
        btnArray[i].glitchFilter = BTN_GLITCH_FILTER_INIT_CNT;
        btnArray[i].btnIsDetectFlag = FALSE;
    }
}

/*******************************************************************************
* Function Name: ButtonOnDetect
********************************************************************************
* Summary:
*   Detect button status for pressing event
*
* Parameters:  
*   void
*
* Return: 
*   void
*
*******************************************************************************/
void ButtonOnDetect(uint8 btnStatus, Btn_Status_T* btnArrayPtr, uint8* btnStatusPtr)
{
    uint8 curBtnStatus = btnStatus;    /* get current button level status */
    if(btnArrayPtr->btnIsDetectFlag == FALSE)          
    {
        /* detect button pressing event */
        btnArrayPtr->btnIsDetectFlag = ((curBtnStatus == BTN_ON_LEVEL) && 
                                       (curBtnStatus ^ btnArrayPtr->preBtnStatus)) ? TRUE : FALSE;
    }
    else
    {
        if(curBtnStatus == BTN_ON_LEVEL)   /* button keeps in desired level status */
        {            
            /*  detect if glitch filter counter value is larger than pre-defined threshold */
            if(btnArrayPtr->glitchFilter > BTN_GLITCH_FILTER_ACTIVE_CNT)
            {
                /* succeed in button pressing detection, set button ON status */
                *btnStatusPtr = BUTTON_ON;
                /* reset glitch filter coutner */
                btnArrayPtr->glitchFilter = BTN_GLITCH_FILTER_INIT_CNT;
                /* clear detection flag */
                btnArrayPtr->btnIsDetectFlag = FALSE;
            }
            else
            {
                /* increase glitch filter coutner */
                btnArrayPtr->glitchFilter++; 
            }
        }
        else
        {
            if(btnArrayPtr->glitchFilter < BTN_GLITCH_FILTER_DISCARD_CNT)
            {
                /* keep buttons status unchanged, reset glitch filter coutner */
                btnArrayPtr->glitchFilter = BTN_GLITCH_FILTER_INIT_CNT;
                /* clear detection flag */
                btnArrayPtr->btnIsDetectFlag = FALSE;
            }
            else
            {
                 /* decrease glitch filter coutner */
                btnArrayPtr->glitchFilter--; 
            }
        }
    }
    /* update last button status with current status */
    btnArrayPtr->preBtnStatus = curBtnStatus;
}

/*******************************************************************************
* Function Name: ButtonOffDetect
********************************************************************************
* Summary:
*   Detect button status for releasing event
*
* Parameters:  
*   void
*
* Return: 
*   void
*
*******************************************************************************/
void ButtonOffDetect(uint8 btnStatus, Btn_Status_T* btnArrayPtr, uint8* btnStatusPtr)
{
    uint8 curBtnStatus = btnStatus;    /* get current button level status */
    if(btnArrayPtr->btnIsDetectFlag == FALSE)          
    {
        /* detect button releasing event */
        btnArrayPtr->btnIsDetectFlag = ((curBtnStatus == BTN_OFF_LEVEL) && 
                                       (curBtnStatus ^ btnArrayPtr->preBtnStatus)) ? TRUE : FALSE;
    }
    else
    {
        if(curBtnStatus == BTN_OFF_LEVEL)   /* button keeps in desired level status */
        {            
            /*  detect if glitch filter counter value is larger than pre-defined threshold */
            if(btnArrayPtr->glitchFilter > BTN_GLITCH_FILTER_ACTIVE_CNT)
            {
                /* succeed in button releasing detection, set button OFF status */
                *btnStatusPtr = BUTTON_OFF;
                /* reset glitch filter coutner */
                btnArrayPtr->glitchFilter = BTN_GLITCH_FILTER_INIT_CNT;
                /* clear detection flag */
                btnArrayPtr->btnIsDetectFlag = FALSE;
            }
            else
            {
                /* increase glitch filter coutner */
                btnArrayPtr->glitchFilter++; 
            }
        }
        else
        {
            if(btnArrayPtr->glitchFilter < BTN_GLITCH_FILTER_DISCARD_CNT)
            {
                /* keep buttons status unchanged, reset glitch filter coutner */
                btnArrayPtr->glitchFilter = BTN_GLITCH_FILTER_INIT_CNT;
                /* clear detection flag */
                btnArrayPtr->btnIsDetectFlag = FALSE;
            }
            else
            {
                 /* decrease glitch filter coutner */
                btnArrayPtr->glitchFilter--; 
            }
        }
    }
    /* update last button status with current status */
    btnArrayPtr->preBtnStatus = curBtnStatus;
}

/*******************************************************************************
* Function Name: ButtonProcess
********************************************************************************
* Summary:
* The UpdateStatusStart function implements start function.
* voltage. 
*
* Parameters:  
*  void:  
*
* Return: 
*  void
*
*******************************************************************************/
void ButtonProcess()
{
    /* scan SW1 button status */
    if(btnStatus[0] == BUTTON_OFF)
        ButtonOnDetect(SW2_Read(), &btnArray[0], &btnStatus[0]);
    else
        ButtonOffDetect(SW2_Read(), &btnArray[0], &btnStatus[0]);
}


/* [] END OF FILE */

