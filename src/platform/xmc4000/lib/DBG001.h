/*******************************************************************************
**                                                                            **
** Copyright (C) Infineon Technologies (2011)                                 **
**                                                                            **
** All rights reserved.                                                       **
**                                                                            **
** This document contains proprietary information belonging to Infineon       **
** Technologies. Passing on and copying of this document, and communication   **
** of its contents is not permitted without prior written authorization.      **
**                                                                            **
********************************************************************************
**                                                                            **
**                                                                            **
** PLATFORM : Infineon XMC4000 Series                           			  **
**                                                                            **
** COMPILER : Compiler Independent                                            **
**                                                                            **
** AUTHOR : App Developer                                                     **
**                                                                            **
** MAY BE CHANGED BY USER [yes/no]: Yes                                       **
**                                                                            **
** MODIFICATION DATE : Mar 8, 2011                                            **
**                                                                            **
*******************************************************************************/
/**
 * @file DebugLog.h
 *
 * @brief  Header file for DebugLog.
 *
 */
#ifndef DEBUGLOG_H_CONFIG
#define DEBUGLOG_H_CONFIG
/*******************************************************************************
**                                  File Include                              **
*******************************************************************************/
/* Inclusion of standard types */
#include <DAVE3.h>


/* Inclusion of definitions about Invariant Checking */
//#include "nana.h"

/*******************************************************************************
**                      Global Macro Definitions                              **
*******************************************************************************/

/** GroupID of APPs */
typedef enum DebugLog_GroupIDType
{
	GID_DBGLOG = 0,	/* # DEBUGLOG 			*/
	GID_HWEXCP,		/* # HARDWARE EXCEPTION */
	GID_XSPY ,			/* # XSPY 				*/
	GID_UART001,		/* # USIC UART 		*/
	GID_UART002,		/* # USIC UART OS 		*/
	GID_I2C001,			/* # USIC_I2C 		*/
	GID_I2C002,			/* # USIC_I2COS 		*/
	GID_SPI001,			/* # USIC_SPI 		*/
	GID_SPI002,			/* # USIC_SPIOS 		*/
	GID_I2S001,			/* # USIC_I2S 		*/
	GID_CAN001,			/* # SIMPLE CAN		*/
	GID_CAN002,			/* # CAN RTOS 		*/
	GID_DMA002,				/* # ADVANCED DMA 		*/
	GID_DMA003,				/* # STANDARD DMA 		*/
	GID_DMA004,				/* # SOFTWARE DMA		*/
	GID_NVIC001,				/* # NVIC GENERIC 		*/
	GID_NVIC002,				/* # NVIC EMPTY */
	GID_NVIC003,				/* # NVIC SCU 	*/
	GID_NVIC004,				/* # NVIC DMA 	*/
	GID_IO001,				/* # ANALOG IO 		*/
	GID_IO002,				/* # DIGITAL IO 		*/
	GID_IO003,				/* # PORT IO 		*/
	GID_ERU001,				/* # ERU  		*/
	GID_ERU002,				/* # ERU 		*/
	GID_TMPS001,				/* # DIE TEMP 		*/
	GID_POW001,				/* # STATE CONTROL 		*/
	GID_CLK001,				/* # CLOCK  		*/
	GID_EXCP001,				/* # HARDWARE EXCEPTION 		*/
	GID_SYSTM001,				/* # SYSTEM TIMER 		*/
	GID_RTC001,				/* # RTC 		*/
	GID_EBU001,				/* # EXTERNAL BUS UNIT 		*/
  	GID_FLASH001 ,				/* # SIMPLE FLASH		*/
  	GID_FLASH002 ,				/* # INTERNAL FLASH 		*/
  	GID_GEN001 ,				/* # CSTART 		*/
  	GID_WDT001 ,				/* # WATCHDOG 		*/
  	GID_ETH009 ,				/* # MODBUS 		*/
  	GID_RESET001 ,				/* # RESET 		*/
  	GID_ETHPHY,				/* # ETHERNET PHY 		*/
	GID_SIMPLECAN,			/* # SIMPLE CAN 		*/
	GID_POSHE001,			/* # HALL SENSOR 		*/
	GID_POSQE001,			/* # QUADRATURE DECODER */
	GID_CNT001,		/* # EVENT COUNTER 		*/
	GID_CAP001,   /* # CAPTURE DUTY CYCLE AND PERIOD */
	GID_PWMSP001, /* # SINGLE PHASE PWM APP */
	GID_PWMSP002, /* #SINGLE PHASE PWM WITH DEAD TIME */
	GID_PWMMP001, /* #MULTIPHASE PWM */
	GID_RTOSWRAPPER,		/* # RTOS WRAPPER 		*/
	GID_HWCRC,				/* # HW CRC 			*/
	GID_SWCRC,				/* # SW CRC 			*/
	GID_USB,				/* # USB				*/
	GID_SDMMCLLD,    /* #SDMMC LLD*/
	GID_SDMMCBLOCKLAYER,    /* #SDMMC Block layer */
	GID_DAL,                 /*Sdmmc Device Abstraction Layer */
	GID_GUI,					/* GUI App*/			
	GID_KEYBOARD,     /* GUI Keyboard App*/
	GID_MOUSE,        /* GUI Mouse  App*/
	GID_TOUCHSCREEN,
	GID_SYSTM,           /* Simple system timer */
	GID_END = 256
}DebugLog_GroupIDType;

/**
 * @addtogroup DebugLog_publicparam
 * @{
 */
/** Macro for Logging function start */
#define FUNCTION_ENTRY(GID, Status)

/** Macro for logging function stop */
#define FUNCTION_EXIT(GID, Status)


/*******************************************************************************
**                      Global Type Definitions                               **
*******************************************************************************/

/**
 * @brief Typedef for the Callback
 */
typedef status_t (*CmdCallback)(void*);

/**
 * @brief Enumeration for Error Codes
 */
typedef enum DebugLog_ErrorType
{
  DEBUGLOG_ERROR = 1,		/* # DebugLog Error				*/
  DEBUGLOG_PBC,				/* # Progamming By Contract		*/
  DEBUGLOG_TRANSPORT_BUSY,	/* # Physical trasport is busy  */
}
DebugLog_ErrorType;

#if 0
typedef enum DebugLog_SeverityLevelType
{
   /** Severity Level Not Set */
   DEBUGLOG_LEVEL_NOTSET,
   /** Debug Message sent with this is considered as TRACE Message */
   DEBUGLOG_LEVEL_TRACE,
   /** Debug Message sent with this is considered as INFO Message */
   DEBUGLOG_LEVEL_INFO,
   /** Debug Message sent with this is considered as WARNING Message */
   DEBUGLOG_LEVEL_WARNING,
   /** Debug Message sent with this is considered as ERROR Message */
   DEBUGLOG_LEVEL_ERROR,
   /** Debug Message sent with this is considered as CRITICAL Message */
   DEBUGLOG_LEVEL_CRITICAL,
   /** Debug Message sent with this is considered as SAFETY CRITICAL Message */
   DEBUGLOG_LEVEL_SAFETY_CRITICAL
}DebugLog_SeverityLevelType;
#else
/** Enumeration for Severity Type - LEVEL NOT SET */
#define DEBUGLOG_LEVEL_NOTSET           0
/** Enumeration for Severity Type - SET TO TRACE */
#define DEBUGLOG_LEVEL_TRACE            1
/** Enumeration for Severity Type - SET TO INFO */
#define DEBUGLOG_LEVEL_INFO             2
/** Enumeration for Severity Type - SET TO WARNING */
#define DEBUGLOG_LEVEL_WARNING          3
/** Enumeration for Severity Type - SET TO ERROR */
#define DEBUGLOG_LEVEL_ERROR            4
/** Enumeration for Severity Type - SET TO CRITICAL */
#define DEBUGLOG_LEVEL_CRITICAL         5
/** Enumeration for Severity Type - SET TO SAFETY CRITICAL */
#define DEBUGLOG_LEVEL_SAFETY_CRITICAL  6
#endif

/**
 * @brief Enumeration for type of Physical Interface
 */
typedef enum DebugLog_PhyicalInterfaceType
{
  /** UART */
   UART,
   /** Simple Controller Area Network */
   SIMPLE_CAN,
   /** Ethernet */
   ETHERNET,
   /** File System */
   FILESYSTEM
}DebugLog_PhyicalInterfaceType;

/**
 * @brief Enumeration for type of Static Filter Options
 */
typedef enum DebugLog_StaticFilterOptionsType
{
  /** EMPTY: No message will be logged */
   DEBUGLOG_EMPTY,
   /** TRUE: All the messages will be logged */
   DEBUGLOG_TRUE,
   /** MACRO: Filtering will be decided by the macro formed by GroupID
    * and Severity Level set in the GUI */
   DEBUGLOG_FILTERING_MACRO
}DebugLog_StaticFilterOptionsType;

/**
 * @brief Enumeration for type of Run time Filter
 */
typedef enum DebugLog_RuntimeFilterType
{
  /** Static Runtime Filter */
   DEBUGLOG_LEVEL_STATIC,
   /** Dynamic Run Time Filtering */
   DEBUGLOG_LEVEL_DYNAMIC
}DebugLog_RuntimeFilterType;

/**
 * @brief Enumeration for type of Assertion Type
 */
typedef enum DebugLog_InvarientAssertionType
{
  /** Assert always */
  ALWAYS_ASSERT,
  /** assert depends upon the condition */
  CONDITIONAL_ASSERT,
  /** Never assert */
  NO_ASSERT
}DebugLog_InvarientAssertionType;

/**
 * @brief Enumeration for Assert Options
 */
typedef enum DebugLog_AssertOptionType
{
  /** Stop the working */
  BREAK_INSTRUCTION,
  /** sends the debug message to host */
  SENDTO_DEBUGLOG
}DebugLog_AssertOptionType;

/**
 * @}
 */

/**
 * @addtogroup DebugLog_appconfigdoc
 * @{
 */

#ifndef AUTOMATED_TESTING
/** GUI Configurable element,   Number of Channels utilized in the DeugLog */
#define NO_OF_CHANNELS          1

/** severity level */
#define COMPILE_TIME_FILTER     DEBUGLOG_LEVEL_NOTSET

/** GUI Configurable element, PBC severity level */
#define PBC_SEVERITY            DEBUGLOG_LEVEL_WARNING

/** GUI Configurable element, no assertion */
#define INVARIENT_ASSERTION     NO_ASSERT

/** GUI Configurable element, send the messages to DebugLog*/
#define ASSERT_OPTIONS          SENDTO_DEBUGLOG

#else
//#include "debuglog_it.h"
#include "topapp.h"
/** GUI Configurable element, Number of Channels utilized in the DeugLog */
#define NO_OF_CHANNELS          GUI_NUM_CHANNELS

/** severity level */
#define COMPILE_TIME_FILTER     GUI_COMPILE_TIME_FILTER

/** GUI Configurable element, PBC severity level */
#define PBC_SEVERITY            GUI_PBC_SEVERITY

/** GUI Configurable element, no assertion */
#define INVARIENT_ASSERTION     GUI_INVARIENT_ASSERTION

/** GUI Configurable element, send the messages to DebugLog*/
#define ASSERT_OPTIONS          GUI_ASSERT_OPTIONS


#endif

/**
 * @brief Format of Command sent by the Host
 */
typedef struct DebugLog_CommandFromHostType
{
    /** Group ID: ID of the application for which the command is intended for */
    uint8_t GroupID;
	/** Counter: wrap around counter used for detecting the loss of messages */
    uint8_t Counter;
    /** Message Length: Length of the command message which will follow */
    uint8_t MessageLen;
    /** Message: Actual Message (LSByte First) */
    uint8_t Message[256];
}DebugLog_CommandFromHostType;

/**
 * @brief Function pointer for the Physical Interface Init Function
 */
typedef status_t (*PhyInterfaceInitFuncPtr)(void*);

/**
 * @brief Function pointer for the Physical Interface Write Function
 */
typedef status_t (*PhyInterfaceWriteFuncPtr)(void*,uint32_t,uint8_t*);



/**
 * @brief GUI configurable elements for every channel
 */
typedef struct DebugLog_GUIConfigurableType
{
  /** gid bit mask */
  uint32_t                          ChannelGroupIDMask[8];
  /** static/dynamic */
  DebugLog_RuntimeFilterType        RunTimeFilter;
  /** empty/true/macro */
  DebugLog_StaticFilterOptionsType  RunTimeStaticFilter;
  /** gid for dynamic filtering */
  uint32_t                          StaticFilterMacroGroupID[8];
  /** severity level for dynamic filter, for static filtering - macro option */
  uint32_t                          RunTimeDynamicFilterSeverity;
  /** circular buffer size */
  uint8_t                           CircularBufferSize;
  /** debug message of this severity initiates the dump */
  uint32_t                          SeverityToDump;
  /** physical channel */
  DebugLog_PhyicalInterfaceType     PhysicalChannel;
  /** Physical Interface Handle */
  handle_t                          PeripheralHandle;

  /** Pointer to the physical Interface Init function */
  PhyInterfaceInitFuncPtr           PhysicalInterfaceInit;

  /** Pointer to the Physical Interface Write function */
  PhyInterfaceWriteFuncPtr			PhysicalInterfaceWrite;

  DebugLog_CommandFromHostType      CommandFromHost;

}DebugLog_GUIConfigurableType;

/**
 * @}
 */

/*******************************************************************************
**                          FUNCTION PROTOTYPES                               **
*******************************************************************************/
/** @addtogroup DebugLog_apidoc
 * @{
 */

/**
 * @brief Initializes the debuglog app's global data structures and registers
 * callback.
 *
 * Service ID:  0x01
 *
 * @return      None
 * <b>Reentrant: yes</b><BR>
 * <b>Sync/Async:  Synchronous</b>
 *
 */
void DebugLog_Init(void);

/**
 * @brief       The APP's uses this API to register their callbacks with
 *              debuglog. Debuglog will call the appropriate callback and
 *              forwards the message from the host.
 *
 * Service ID:  0x02
 *
 * @param[in]   groupid      Application ID
 * @param[in]   Callback     A callback to register.
 *
 * @return      status_t
 * <b>Reentrant: yes</b><BR>
 * <b>Sync/Async:  Synchronous</b>
 *
 */

status_t DebugLog_RegisterCallBack
(
  uint8_t groupid,
  CmdCallback Callback
);

/**
 * @brief       This API is mapped to the MACROS. The first parameter is
 *              appended by the macro.
 *
 * Service ID:  0x01
 *
 * @param[in]   Severity_Level  Severity level of the MEssage
 * @param[in]   GroupID        Application ID
 * @param[in]   MessageID      Message / Error ID
 * @param[in]   Length         Length of the Message
 * @param[in]   Message        Pointer to the Message
 *
 * @return     None
 * <b>Reentrant: yes</b><BR>
 * <b>Sync/Async:  Synchronous</b>
 *
 */
void DebugLog_Logging
(
  uint32_t Severity_Level,
  uint8_t GroupID,
  uint8_t MessageID,
  uint8_t Length,
  const uint8_t* Message
);

status_t DebugLog_UARTInit(void* Handle);
status_t DebugLog_UARTWrite(void* Handle, uint32_t Len, uint8_t* Buffer);
status_t DebugLog_UARTCallBack(void* Param);

/* Compile time Filtering MACROS */
#if (COMPILE_TIME_FILTER == DEBUGLOG_LEVEL_TRACE)
  /** */
  #define SAFETY_CRITICAL(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_SAFETY_CRITICAL,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define CRITICAL(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_CRITICAL,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define ERROR(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_ERROR,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define WARNING(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_WARNING,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define INFO(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_INFO,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define TRACE(groupid,messageid,length,value)\
          DebugLog_Logging(DEBUGLOG_LEVEL_TRACE,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
#elif (COMPILE_TIME_FILTER == DEBUGLOG_LEVEL_INFO)
  /** */
  #define SAFETY_CRITICAL(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_SAFETY_CRITICAL,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define CRITICAL(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_CRITICAL,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define ERROR(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_ERROR,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define WARNING(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_WARNING,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define INFO(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_INFO,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define TRACE(groupid,messageid,length,value)\
          /** NULL */
#elif (COMPILE_TIME_FILTER == DEBUGLOG_LEVEL_WARNING)
  /** */
  #define SAFETY_CRITICAL(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_SAFETY_CRITICAL,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define CRITICAL(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_CRITICAL,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define ERROR(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_ERROR,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define WARNING(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_WARNING,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define INFO(groupid,messageid,length,value) \
          /** NULL */
  /** */
  #define TRACE(groupid,messageid,length,value)\
          /** NULL */
#elif (COMPILE_TIME_FILTER == DEBUGLOG_LEVEL_ERROR)
  /** */
  #define SAFETY_CRITICAL(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_SAFETY_CRITICAL,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define CRITICAL(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_CRITICAL,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define ERROR(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_ERROR,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define WARNING(groupid,messageid,length,value) \
          /** NULL */
  /** */
  #define INFO(groupid,messageid,length,value) \
          /** NULL */
  /** */
  #define TRACE(groupid,messageid,length,value)\
          /** NULL */
#elif (COMPILE_TIME_FILTER == DEBUGLOG_LEVEL_CRITICAL)
  /** */
  #define SAFETY_CRITICAL(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_SAFETY_CRITICAL,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define CRITICAL(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_CRITICAL,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define ERROR(groupid,messageid,length,value) \
          /** NULL */
  /** */
  #define WARNING(groupid,messageid,length,value) \
          /** NULL */
  /** */
  #define INFO(groupid,messageid,length,value) \
          /** NULL */
  /** */
  #define TRACE(groupid,messageid,length,value)\
          /** NULL */
#elif (COMPILE_TIME_FILTER == DEBUGLOG_LEVEL_SAFETY_CRITICAL)
  /** */
  #define SAFETY_CRITICAL(groupid,messageid,length,value) \
          DebugLog_Logging(DEBUGLOG_LEVEL_SAFETY_CRITICAL,(uint8_t)groupid,\
                           (uint8_t)messageid, (uint8_t)length,(uint8_t*)value);
  /** */
  #define CRITICAL(groupid,messageid,length,value) \
          /** NULL */
  /** */
  #define ERROR(groupid,messageid,length,value) \
          /** NULL */
  /** */
  #define WARNING(groupid,messageid,length,value) \
          /** NULL */
  /** */
  #define INFO(groupid,messageid,length,value) \
          /** NULL */
  /** */
  #define TRACE(groupid,messageid,length,value)\
          /** NULL */
  #elif (COMPILE_TIME_FILTER == DEBUGLOG_LEVEL_NOTSET)
  /** */
  #define SAFETY_CRITICAL(groupid,messageid,length,value)
          /** NULL */
  /** */
  #define CRITICAL(groupid,messageid,length,value)
          /** NULL */
  /** */
  #define ERROR(groupid,messageid,length,value)
          /** NULL */
  /** */
  #define WARNING(groupid,messageid,length,value)
          /** NULL */
  /** */
  #define INFO(groupid,messageid,length,value)
          /** NULL */
  /** */
  #define TRACE(groupid,messageid,length,value)
          /** NULL */
#endif

/**
 * @}
 */
extern DebugLog_GUIConfigurableType ChannelConfig[NO_OF_CHANNELS];
#endif /* DEBUGLOG_H_CONFIG */
