/**
  ******************************************************************************
  * @file    lcd_log_conf.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   lcd_log configuration template file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef  __LCD_LOG_CONF_H__
#define  __LCD_LOG_CONF_H__
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

#if defined (USE_STM322xG_EVAL)
 #include "stm322xg_eval_lcd.h"
#elif defined(USE_STM324xG_EVAL)
 #include "stm324xg_eval_lcd.h"
#elif defined (USE_STM3210C_EVAL)
 #include "stm3210c_eval_lcd.h"
#else
 #error "Missing define: Evaluation board (ie. USE_STM322xG_EVAL)"
#endif

/** @addtogroup LCD_LOG
  * @{
  */
  
/** @defgroup LCD_LOG
  * @brief This file is the 
  * @{
  */ 


/** @defgroup LCD_LOG_CONF_Exported_Defines
  * @{
  */ 

/* Comment the line below to disable the scroll back and forward features */
//#define     LCD_SCROLL_ENABLED
            
/* Define the LCD default text color */
#define     LCD_LOG_DEFAULT_COLOR    White

/* Define the display window settings */
#define     YWINDOW_MIN         3
#define     YWINDOW_SIZE        9
#define     XWINDOW_MAX         50

/* Define the cache depth */
#define     CACHE_SIZE          50

/** @defgroup LCD_LOG_CONF_Exported_TypesDefinitions
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup LCD_LOG_Exported_Macros
  * @{
  */ 


/**
  * @}
  */ 

/** @defgroup LCD_LOG_CONF_Exported_Variables
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup LCD_LOG_CONF_Exported_FunctionsPrototype
  * @{
  */ 

/**
  * @}
  */ 


#endif //__LCD_LOG_CONF_H__

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
