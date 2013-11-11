/**
  @page USBD_VCP  USB Device VCP example
  
  @verbatim
  ******************** (C) COPYRIGHT 2012 STMicroelectronics *******************
  * @file    readme.txt 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    19-March-2012
  * @brief   Description of the USB Device VCP (Virtual Com Port) example
  ******************************************************************************
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
  * limitations under the Licens
  *   
  ******************************************************************************
   @endverbatim

   
@par Example Description 

This Demo presents the implementation of a Virtual Com Port (VCP) capability in 
the STM32F2xx, STM32F4xx and STM32F105/7 devices. 

It illustrates an implementation of the CDC class following the PSTN subprotocol.
The VCP example allows the STM32 device to behave as a USB-to-RS232 bridge.
 - On one side, the STM32 communicates with host (PC) through USB interface in Device mode.
 - On the other side, the STM32 communicates with other devices (same host, other host,
   other devices…) through the USART interface (RS232).

The support of the VCP interface is managed through the ST Virtual Com Port driver
available for download from www.st.com.
This example can be customized to communicate with interfaces other than USART.
The VCP example works in High and Full speed modes.

When the VCP application starts, the USB device is enumerated as serial communication
port and can be configured in the same way (baudrate, data format, parity, stop bit
length…).
To test this example, you can use one of the following configurations:
 - Configuration 1: Connect USB cable to host and USART (RS232) to a different host
   (PC or other device) or to the same host. In this case, you can open two hyperterminal-like
   terminals to send/receive data to/from host to/from device.
 - Configuration 2: Connect USB cable to Host and connect USART TX pin to USART
   RX pin on the evaluation board (Loopback mode). In this case, you can open one
   terminal (relative to USB com port or USART com port) and all data sent from this
   terminal will be received by the same terminal in loopback mode. This mode is useful
   for test and performance measurements.

To use a different CDC communication interface you can use the template CDC 
interface provided in folder \Libraries\STM32_USB_Device_Library\Class\cdc.

@note When transferring a big file (USB OUT transfer) user have to adapt the size
      of IN buffer, for more details refer to usbd_conf.h file (APP_RX_DATA_SIZE constant).


This example works
  - in high speed (HS) when the STM322xG-EVAL or the STM324xG-EVAL board and the
    USB OTG HS peripheral are used
  - in full speed (FS) when the STM322xG-EVAL or the STM324xG-EVAL board and the
    USB OTG FS peripheral are used, or when using the STM3210C-EVAL board.
    

@par Hardware and Software environment 

   - This example runs on STM32F105/7 Connectivity line, STM32F2xx and STM32F4xx devices.
  
   - This example has been tested with STM3210C-EVAL RevB (STM32F105/7 devices), 
     STM322xG-EVAL RevB (STM32F2xx) and STM324xG-EVAL RevB (STM32F4xx)   

  - STM3210C-EVAL Setup 
    - Use CN2 connector to connect the board to a PC host
    - Use CN6 connector to connect the board to PC (or to another evaluation board) over 
      RS232 (USART) serial cable.
    - Jumper JP16 should be connected in position 2-3.
    - For loopback mode test: remove RS232 cable on CN6 and connect directly USART
      TX and RX pins: PD5 and PD6 (with a cable or a jumper)
       
  - STM322xG-EVAL Setup
    - Use CN8 connector to connect the board to a PC host when using USB OTG FS peripheral
    - Use CN9 connector to connect the board to a PC host when using USB OTG HS peripheral
    - Use CN16 connector to connect the board to PC (or to another evaluation board) over 
      RS232 (USART) serial cable.
    - Jumper JP22 should be connected in position 1-2.
    - For loopback mode test: remove RS232 cable on CN16 and connect directly USART
      TX and RX pins: PC10 and PC11 (with a cable or a jumper)

  - STM324xG-EVAL Setup
    - Use CN8 connector to connect the board to a PC host when using USB OTG FS peripheral
    - Use CN9 connector to connect the board to a PC host when using USB OTG HS peripheral
    - Use CN16 connector to connect the board to PC (or to another evaluation board) over 
      RS232 (USART) serial cable.
    - Jumper JP22 should be connected in position 1-2.
    - For loopback mode test: remove RS232 cable on CN16 and connect directly USART
      TX and RX pins: PC10 and PC11 (with a cable or a jumper)      
  
  
@par How to use it ?

 + EWARM
    - Open the usbd_vcp.eww workspace.
    - In the workspace toolbar select the project config:
     - STM322xG-EVAL_USBD-HS: to configure the project for STM32F2xx devices and use USB OTG HS peripheral
     - STM322xG-EVAL_USBD-FS: to configure the project for STM32F2xx devices and use USB OTG FS peripheral
     - STM324xG-EVAL_USBD-HS: to configure the project for STM32F4xx devices and use USB OTG HS peripheral
     - STM324xG-EVAL_USBD-FS: to configure the project for STM32F4xx devices and use USB OTG FS peripheral
     - STM3210C-EVAL_USBD-FS: to configure the project for STM32F105/7 devices  
    - Rebuild all files: Project->Rebuild all
    - Load project image: Project->Debug
    - Run program: Debug->Go(F5)

 + MDK-ARM
    - Open the usbd_vcp.uvproj project
    - In the build toolbar select the project config:
     - STM322xG-EVAL_USBD-HS: to configure the project for STM32F2xx devices and use USB OTG HS peripheral
     - STM322xG-EVAL_USBD-FS: to configure the project for STM32F2xx devices and use USB OTG FS peripheral
     - STM324xG-EVAL_USBD-HS: to configure the project for STM32F4xx devices and use USB OTG HS peripheral
     - STM324xG-EVAL_USBD-FS: to configure the project for STM32F4xx devices and use USB OTG FS peripheral
     - STM3210C-EVAL_USBD-FS: to configure the project for STM32F105/7 devices  
    - Rebuild all files: Project->Rebuild all target files
    - Load project image: Debug->Start/Stop Debug Session
    - Run program: Debug->Run (F5)    

 + RIDE
    - Open the usbd_vcp.rprj project.
    - In the configuration toolbar(Project->properties) select the project config:
     - STM322xG-EVAL_USBD-HS: to configure the project for STM32F2xx devices and use USB OTG HS peripheral
     - STM322xG-EVAL_USBD-FS: to configure the project for STM32F2xx devices and use USB OTG FS peripheral
     - STM324xG-EVAL_USBD-HS: to configure the project for STM32F4xx devices and use USB OTG HS peripheral
     - STM324xG-EVAL_USBD-FS: to configure the project for STM32F4xx devices and use USB OTG FS peripheral
     - STM3210C-EVAL_USBD-FS: to configure the project for STM32F105/7 devices  
    - Rebuild all files: Project->build project
    - Load project image: Debug->start(ctrl+D)
    - Run program: Debug->Run(ctrl+F9)

 + TASKING
    - Open TASKING toolchain.
    - Click on File->Import, select General->'Existing Projects into Workspace' 
      and then click "Next". 
    - Browse to  TASKING workspace directory and select the project: 
     - STM322xG-EVAL_USBD-HS: to configure the project for STM32F2xx devices and use USB OTG HS peripheral
     - STM322xG-EVAL_USBD-FS: to configure the project for STM32F2xx devices and use USB OTG FS peripheral
     - STM324xG-EVAL_USBD-HS: to configure the project for STM32F4xx devices and use USB OTG HS peripheral
     - STM324xG-EVAL_USBD-FS: to configure the project for STM32F4xx devices and use USB OTG FS peripheral
     - STM3210C-EVAL_USBD-FS: to configure the project for STM32F105/7 devices  
   
    - Rebuild all project files: Select the project in the "Project explorer" 
      window then click on Project->build project menu.
    - Run program: Select the project in the "Project explorer" window then click 
      Run->Debug (F11)

 + TrueSTUDIO
    - Open the TrueSTUDIO toolchain.
    - Click on File->Switch Workspace->Other and browse to TrueSTUDIO workspace 
      directory.
    - Click on File->Import, select General->'Existing Projects into Workspace' 
      and then click "Next". 
    - Browse to the TrueSTUDIO workspace directory and select the project:  
     - STM322xG-EVAL_USBD-HS: to configure the project for STM32F2xx devices and use USB OTG HS peripheral
     - STM322xG-EVAL_USBD-FS: to configure the project for STM32F2xx devices and use USB OTG FS peripheral
     - STM324xG-EVAL_USBD-HS: to configure the project for STM32F4xx devices and use USB OTG HS peripheral
     - STM324xG-EVAL_USBD-FS: to configure the project for STM32F4xx devices and use USB OTG FS peripheral
     - STM3210C-EVAL_USBD-FS: to configure the project for STM32F105/7 devices  
    - Rebuild all project files: Select the project in the "Project explorer" 
      window then click on Project->build project menu.
    - Run program: Select the project in the "Project explorer" window then click 
      Run->Debug (F11)
    
@note Known Limitations
      This example retargets the C library printf() function to the EVAL board’s LCD
      screen (C library I/O redirected to LCD) to display some Library and user debug
      messages. TrueSTUDIO Lite version does not support I/O redirection, and instead
      have do-nothing stubs compiled into the C runtime library. 
      As consequence, when using this toolchain no debug messages will be displayed
      on the LCD (only some control messages in green will be displayed in bottom of
      the LCD). To use printf() with TrueSTUDIO Professional version, just include the
      TrueSTUDIO Minimal System calls file "syscalls.c" provided within the toolchain.
      It contains additional code to support printf() redirection.
    
 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
