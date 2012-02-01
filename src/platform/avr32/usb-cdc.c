/* Copyright (c) 2009 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE
 */

#include "platform_conf.h"

#ifdef BUILD_USB_CDC

#include <string.h>
#include "compiler.h"
#include "pm.h"
#include "intc.h"
#include "usart.h"
#include "usb-cdc.h"

volatile       U16                                g_usb_event = 0;
volatile       Bool                               usb_connected = FALSE;
volatile       U8                                 usb_configuration_nb=0;
S_line_coding                                     line_coding;
const          void                               *pbuffer;
               U16                                 data_to_transfer;
static         U8                                  bmRequestType;
extern  const  S_usb_device_descriptor             usb_user_device_descriptor;
extern  const  S_usb_user_configuration_descriptor usb_user_configuration_descriptor;
static         U8                                  usb_interface_status[NB_INTERFACE];  // All interface with default setting

volatile Bool b_tx_new;
volatile Bool b_rx_new;

UnionVPtr pep_fifo[MAX_PEP_NB];

void usb_init(void)
{
  b_tx_new = TRUE;
  b_rx_new = TRUE;
  usb_connected = FALSE;
  usb_configuration_nb = 0;

  Disable_global_interrupt();
  INTC_register_interrupt(&usb_general_interrupt, AVR32_USBB_IRQ, AVR32_INTC_INT0);
  Enable_global_interrupt();
  Usb_force_device_mode();
  Disable_global_interrupt();
  Usb_disable();
  (void)Is_usb_enabled();
  Enable_global_interrupt();
  Usb_disable_otg_pad();
  Usb_enable_otg_pad();
  Usb_enable();
  Usb_unfreeze_clock();
  (void)Is_usb_clock_frozen();
  Usb_ack_suspend();  // A suspend condition may be detected right after enabling the USB macro
  Usb_enable_vbus_interrupt();
  Enable_global_interrupt();
}

//!
//! @brief Entry point of the USB device mamagement
//!
//! This function is the entry point of the USB management. Each USB
//! event is checked here in order to launch the appropriate action.
//! If a Setup request occurs on the Default Control Endpoint,
//! the usb_process_request() function is call in the usb_standard_request.c file
//!
void usb_device_task(void)
{
    if (!usb_connected && Is_usb_vbus_high())
    {
      usb_start_device();
      Usb_send_event(EVT_USB_POWERED);
      Usb_vbus_on_action();
    }

    if (Is_usb_event(EVT_USB_RESET))
    {
      Usb_ack_event(EVT_USB_RESET);
      Usb_reset_endpoint(EP_CONTROL);
      usb_configuration_nb = 0;
    }

    // Connection to the device enumeration process
    if (Is_usb_setup_received())
    {
      usb_process_request();
    }
}

//! @brief USB interrupt routine
//!
//! This function is called each time a USB interrupt occurs.
//! The following USB DEVICE events are taken in charge:
//! - VBus On / Off
//! - Start-of-Frame
//! - Suspend
//! - Wake-Up
//! - Resume
//! - Reset
//!
//! The following USB HOST events are taken in charge:
//! - Device connection
//! - Device Disconnection
//! - Start-of-Frame
//! - ID pin change
//! - SOF (or Keep alive in low-speed) sent
//! - Wake-up on USB line detected
//! - Pipe events
//!
//! For each event, the user can launch an action by completing the associated
//! \#define (see the conf_usb.h file to add actions on events).
//!
//! Note: Only interrupt events that are enabled are processed.
//!
//! Warning: If device and host tasks are not tasks in an RTOS, rough events
//! like ID transition, VBus transition, device disconnection, etc. that need to
//! kill then restart these tasks may lead to an undefined state if they occur
//! just before something is activated in the USB macro (e.g. pipe/endpoint
//! transfer...).
//!
//! @return Nothing in the standalone configuration; a boolean indicating
//!         whether a task switch is required in the FreeRTOS configuration

__attribute__((__interrupt__))
void usb_general_interrupt(void)
{
    // VBus state detection
    if (Is_usb_vbus_transition() && Is_usb_vbus_interrupt_enabled())
    {
      Usb_ack_vbus_transition();
      if (Is_usb_vbus_high())
      {
        usb_start_device();
        Usb_send_event(EVT_USB_POWERED);
        Usb_vbus_on_action();
      }
      else
      {
        Usb_unfreeze_clock();
        Usb_detach();
        usb_connected = FALSE;
        usb_configuration_nb = 0;
        Usb_send_event(EVT_USB_UNPOWERED);
        Usb_vbus_off_action();
      }
    }
    // Device Start-of-Frame received
    if (Is_usb_sof() && Is_usb_sof_interrupt_enabled())
    {
      Usb_ack_sof();
      //Usb_sof_action();
    }
    // Device Suspend event (no more USB activity detected)
    if (Is_usb_suspend() && Is_usb_suspend_interrupt_enabled())
    {
      Usb_ack_suspend();
      Usb_enable_wake_up_interrupt();
      (void)Is_usb_wake_up_interrupt_enabled();
      Usb_freeze_clock();
      Usb_send_event(EVT_USB_SUSPEND);
      Usb_suspend_action();
    }
    // Wake-up event (USB activity detected): Used to resume
    if (Is_usb_wake_up() && Is_usb_wake_up_interrupt_enabled())
    {
      Usb_unfreeze_clock();
      (void)Is_usb_clock_frozen();
      Usb_ack_wake_up();
      Usb_disable_wake_up_interrupt();
      Usb_wake_up_action();
      Usb_send_event(EVT_USB_WAKE_UP);
    }
    // Resume state bus detection
    if (Is_usb_resume() && Is_usb_resume_interrupt_enabled())
    {
      Usb_disable_wake_up_interrupt();
      Usb_ack_resume();
      Usb_disable_resume_interrupt();
      Usb_resume_action();
      Usb_send_event(EVT_USB_RESUME);
    }
    // USB bus reset detection
    if (Is_usb_reset() && Is_usb_reset_interrupt_enabled())
    {
      Usb_ack_reset();
      usb_init_device();
      Usb_reset_action();
      Usb_send_event(EVT_USB_RESET);
    }
}

//!
//! @brief This function starts the USB device controller.
//!
//! This function enables the USB controller and inits the USB interrupts.
//! The aim is to allow the USB connection detection in order to send
//! the appropriate USB event to the operating mode manager.
//! Start device function is executed once VBus connection has been detected
//! either by the VBus change interrupt or by the VBus high level.
//!
void usb_start_device(void)
{
  Usb_enable_suspend_interrupt();
  Usb_enable_reset_interrupt();

  Usb_force_full_speed_mode();

  usb_init_device();  // Configure the USB controller EP0
  Usb_attach();
  usb_connected = TRUE;
}

//! usb_init_device
//!
//!  This function initializes the USB device controller and
//!  configures usb_get_descriptorthe Default Control Endpoint.
//!
//! @return Status
//!
Status_bool_t usb_init_device(void)
{
  return Is_usb_id_device() && !Is_usb_endpoint_enabled(EP_CONTROL) &&
         Usb_configure_endpoint(EP_CONTROL,
                                AVR32_USBB_UECFG0_EPTYPE_CONTROL,
                                AVR32_USBB_UECFG0_EPDIR_OUT,
                                EP_CONTROL_LENGTH,
                                AVR32_USBB_UECFG0_EPBK_SINGLE);
}


//_____ P R I V A T E   D E C L A R A T I O N S ____________________________

static  void    usb_get_descriptor   (void);
static  void    usb_set_address      (void);
static  void    usb_set_configuration(void);
static  void    usb_clear_feature    (void);
static  void    usb_set_feature      (void);
static  void    usb_get_status       (void);
static  void    usb_get_configuration(void);
static  Bool    usb_get_interface    (void);
static  void    usb_set_interface    (void);


//! This function reads the SETUP request sent to the default control endpoint
//! and calls the appropriate function. When exiting of the usb_read_request
//! function, the device is ready to manage the next request.
//!
//! If the received request is not supported or a non-standard USB request, the function
//! will call the custom decoding function in usb_specific_request module.
//!
//! @note List of supported requests:
//! GET_DESCRIPTOR
//! GET_CONFIGURATION
//! SET_ADDRESS
//! SET_CONFIGURATION
//! CLEAR_FEATURE
//! SET_FEATURE
//! GET_STATUS
//!
void usb_process_request(void)
{
  U8 bRequest;

  Usb_reset_endpoint_fifo_access(EP_CONTROL);
  bmRequestType = Usb_read_endpoint_data(EP_CONTROL, 8);
  bRequest      = Usb_read_endpoint_data(EP_CONTROL, 8);

  switch (bRequest)
  {
  case GET_DESCRIPTOR:
    if (bmRequestType == 0x80) usb_get_descriptor();
    else goto unsupported_request;
    break;

  case GET_CONFIGURATION:
    if (bmRequestType == 0x80) usb_get_configuration();
    else goto unsupported_request;
    break;

  case SET_ADDRESS:
    if (bmRequestType == 0x00) usb_set_address();
    else goto unsupported_request;
    break;

  case SET_CONFIGURATION:
    if (bmRequestType == 0x00) usb_set_configuration();
    else goto unsupported_request;
    break;

  case CLEAR_FEATURE:
    if (bmRequestType <= 0x02) usb_clear_feature();
    else goto unsupported_request;
    break;

  case SET_FEATURE:
    if (bmRequestType <= 0x02) usb_set_feature();
    else goto unsupported_request;
    break;

  case GET_STATUS:
    if (0x7F < bmRequestType && bmRequestType <= 0x82) usb_get_status();
    else goto unsupported_request;
    break;

  case GET_INTERFACE:
    if (bmRequestType == 0x81)
    {
      if(!usb_get_interface())
      {
        Usb_enable_stall_handshake(EP_CONTROL);
        Usb_ack_setup_received_free();
      }
    }
    else goto unsupported_request;
    break;

  case SET_INTERFACE:
    if (bmRequestType == 0x01) usb_set_interface();
    else goto unsupported_request;
    break;

  case SET_DESCRIPTOR:
  case SYNCH_FRAME:
  default:  //!< unsupported request => call to user read request
unsupported_request:
    if (!usb_user_read_request(bmRequestType, bRequest))
    {
      Usb_enable_stall_handshake(EP_CONTROL);
      Usb_ack_setup_received_free();
    }
    break;
  }
}


//! This function manages the SET ADDRESS request. When complete, the device
//! will filter the requests using the new address.
//!
void usb_set_address(void)
{
  U8 addr = Usb_read_endpoint_data(EP_CONTROL, 8);
  Usb_configure_address(addr);

  Usb_ack_setup_received_free();

  Usb_ack_control_in_ready_send();    //!< send a ZLP for STATUS phase
  while (!Is_usb_control_in_ready()); //!< waits for status phase done
                                      //!< before using the new address
  Usb_enable_address();
}


//! This function manages the SET CONFIGURATION request. If the selected
//! configuration is valid, this function call the usb_user_endpoint_init()
//! function that will configure the endpoints following the configuration
//! number.
//!
void usb_set_configuration(void)
{
  U8 configuration_number = Usb_read_endpoint_data(EP_CONTROL, 8);
  U8 u8_i;

  if (configuration_number <= NB_CONFIGURATION)
  {
    Usb_ack_setup_received_free();
    usb_configuration_nb = configuration_number;
    for( u8_i=0; u8_i<NB_INTERFACE; u8_i++) usb_interface_status[u8_i]=0;

    usb_user_endpoint_init(usb_configuration_nb); //!< endpoint configuration
    Usb_set_configuration_action();

    Usb_ack_control_in_ready_send();              //!< send a ZLP for STATUS phase
  }
  else
  {
    //!< keep that order (set StallRq/clear RxSetup) or a
    //!< OUT request following the SETUP may be acknowledged
    Usb_enable_stall_handshake(EP_CONTROL);
    Usb_ack_setup_received_free();
  }
}


//! This function manages the GET DESCRIPTOR request. The device descriptor,
//! the configuration descriptor and the device qualifier are supported. All
//! other descriptors must be supported by the usb_user_get_descriptor
//! function.
//! Only 1 configuration is supported.
//!
void usb_get_descriptor(void)
{
  Bool    zlp;
  U16     wLength;
  U8      descriptor_type;
  U8      string_type;
  Union32 temp;

  zlp             = FALSE;                                  /* no zero length packet */
  string_type     = Usb_read_endpoint_data(EP_CONTROL, 8);  /* read LSB of wValue    */
  descriptor_type = Usb_read_endpoint_data(EP_CONTROL, 8);  /* read MSB of wValue    */

  switch (descriptor_type)
  {
  case DEVICE_DESCRIPTOR:
    data_to_transfer = Usb_get_dev_desc_length();   //!< sizeof(usb_dev_desc);
    pbuffer          = Usb_get_dev_desc_pointer();
    break;

  case CONFIGURATION_DESCRIPTOR:
    data_to_transfer = Usb_get_conf_desc_length();  //!< sizeof(usb_conf_desc);
    pbuffer          = Usb_get_conf_desc_pointer();
    break;

  default:
    if (!usb_user_get_descriptor(descriptor_type, string_type))
    {
      Usb_enable_stall_handshake(EP_CONTROL);
      Usb_ack_setup_received_free();
      return;
    }
    break;
  }

  temp.u32 = Usb_read_endpoint_data(EP_CONTROL, 32);      //!< read wIndex and wLength with a 32-bit access
                                                          //!< since this access is aligned with a 32-bit
                                                          //!< boundary from the beginning of the endpoint
  wLength = usb_format_usb_to_mcu_data(16, temp.u16[1]);  //!< ignore wIndex, keep and format wLength
  Usb_ack_setup_received_free();                          //!< clear the setup received flag

  if (wLength > data_to_transfer)
  {
    zlp = !(data_to_transfer % EP_CONTROL_LENGTH);  //!< zero length packet condition
  }
  else
  {
    // No need to test ZLP sending since we send the exact number of bytes as
    // expected by the host.
    data_to_transfer = wLength; //!< send only requested number of data bytes
  }

  Usb_ack_nak_out(EP_CONTROL);

  while (data_to_transfer && !Is_usb_nak_out(EP_CONTROL))
  {
    while (!Is_usb_control_in_ready() && !Is_usb_nak_out(EP_CONTROL));

    if (Is_usb_nak_out(EP_CONTROL))
      break;  // don't clear the flag now, it will be cleared after

    Usb_reset_endpoint_fifo_access(EP_CONTROL);

    if( 0!= data_to_transfer ) {
       data_to_transfer = usb_write_ep_txpacket(EP_CONTROL, pbuffer,
                                                data_to_transfer, &pbuffer);
    }
    if (Is_usb_nak_out(EP_CONTROL))
      break;

    Usb_ack_control_in_ready_send();  //!< Send data until necessary
  }

  if (zlp && !Is_usb_nak_out(EP_CONTROL))
  {
    while (!Is_usb_control_in_ready());
    Usb_ack_control_in_ready_send();
  }

  while (!Is_usb_nak_out(EP_CONTROL));
  Usb_ack_nak_out(EP_CONTROL);
  while (!Is_usb_control_out_received());
  Usb_ack_control_out_received_free();
}


//! This function manages the GET CONFIGURATION request. The current
//! configuration number is returned.
//!
void usb_get_configuration(void)
{
  Usb_ack_setup_received_free();

  Usb_reset_endpoint_fifo_access(EP_CONTROL);
  Usb_write_endpoint_data(EP_CONTROL, 8, usb_configuration_nb);
  Usb_ack_control_in_ready_send();

  while (!Is_usb_control_out_received());
  Usb_ack_control_out_received_free();
}


//! This function manages the GET STATUS request. The device, interface or
//! endpoint status is returned.
//!
void usb_get_status(void)
{
  U8 wIndex;

  switch (bmRequestType)
  {
  case REQUEST_DEVICE_STATUS:
    Usb_ack_setup_received_free();
    Usb_reset_endpoint_fifo_access(EP_CONTROL);
    Usb_write_endpoint_data(EP_CONTROL, 8, DEVICE_STATUS);
    break;

  case REQUEST_INTERFACE_STATUS:
    Usb_ack_setup_received_free();
    Usb_reset_endpoint_fifo_access(EP_CONTROL);
    Usb_write_endpoint_data(EP_CONTROL, 8, INTERFACE_STATUS);
    break;

  case REQUEST_ENDPOINT_STATUS:
    Usb_read_endpoint_data(EP_CONTROL, 16); //!< dummy read (wValue)
    wIndex = Usb_read_endpoint_data(EP_CONTROL, 8);
    wIndex = Get_desc_ep_nbr(wIndex);
    Usb_ack_setup_received_free();
    Usb_reset_endpoint_fifo_access(EP_CONTROL);
    Usb_write_endpoint_data(EP_CONTROL, 8, Is_usb_endpoint_stall_requested(wIndex) );
    break;

  default:
    Usb_enable_stall_handshake(EP_CONTROL);
    Usb_ack_setup_received_free();
    return;
  }

  Usb_write_endpoint_data(EP_CONTROL, 8, 0x00);
  Usb_ack_control_in_ready_send();

  while (!Is_usb_control_out_received());
  Usb_ack_control_out_received_free();
}


//! This function manages the SET FEATURE request. The USB test modes are
//! supported by this function.
//!
void usb_set_feature(void)
{
  U16 wValue  = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
  U16 wIndex  = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
  U16 wLength = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));

  if (wLength)
    goto unsupported_request;

  switch (wValue)
  {
  case FEATURE_ENDPOINT_HALT:
    wIndex = Get_desc_ep_nbr(wIndex);  // clear direction flag
    if (bmRequestType != ENDPOINT_TYPE ||
        wIndex == EP_CONTROL ||
        !Is_usb_endpoint_enabled(wIndex))
      goto unsupported_request;

    Usb_enable_stall_handshake(wIndex);
    Usb_ack_setup_received_free();
    Usb_ack_control_in_ready_send();
    break;

  case FEATURE_DEVICE_REMOTE_WAKEUP:
  default:
    goto unsupported_request;
  }

  return;

unsupported_request:
  Usb_enable_stall_handshake(EP_CONTROL);
  Usb_ack_setup_received_free();
}


//! This function manages the CLEAR FEATURE request.
//!
void usb_clear_feature(void)
{
  U8 wValue;
  U8 wIndex;

  if (bmRequestType == DEVICE_TYPE || bmRequestType == INTERFACE_TYPE)
  {
    //!< keep that order (set StallRq/clear RxSetup) or a
    //!< OUT request following the SETUP may be acknowledged
    Usb_enable_stall_handshake(EP_CONTROL);
    Usb_ack_setup_received_free();
  }
  else if (bmRequestType == ENDPOINT_TYPE)
  {
    wValue = Usb_read_endpoint_data(EP_CONTROL, 8);

    if (wValue == FEATURE_ENDPOINT_HALT)
    {
      Usb_read_endpoint_data(EP_CONTROL, 8);  //!< dummy read (MSB of wValue)
      wIndex = Usb_read_endpoint_data(EP_CONTROL, 8);
      wIndex = Get_desc_ep_nbr(wIndex);

      if (Is_usb_endpoint_enabled(wIndex))
      {
        if (wIndex != EP_CONTROL)
        {
          Usb_disable_stall_handshake(wIndex);
          Usb_reset_endpoint(wIndex);
          Usb_reset_data_toggle(wIndex);
        }
        Usb_ack_setup_received_free();
        Usb_ack_control_in_ready_send();
      }
      else
      {
        Usb_enable_stall_handshake(EP_CONTROL);
        Usb_ack_setup_received_free();
      }
    }
    else
    {
      Usb_enable_stall_handshake(EP_CONTROL);
      Usb_ack_setup_received_free();
    }
  }
}


//! This function manages the SETUP_GET_INTERFACE request.
//!
Bool usb_get_interface (void)
{
   U16   wInterface;
   U16   wValue;

   // Read wValue
   wValue = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
   // wValue = Alternate Setting
   // wIndex = Interface
   wInterface=usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
   if(0!=wValue)
      return FALSE;
   Usb_ack_setup_received_free();

   Usb_reset_endpoint_fifo_access(EP_CONTROL);
   Usb_write_endpoint_data(EP_CONTROL, 8, usb_interface_status[wInterface] );
   Usb_ack_control_in_ready_send();

   while( !Is_usb_control_out_received() );
   Usb_ack_control_out_received_free();
   return TRUE;
}


//! This function manages the SET INTERFACE request.
//!
void usb_set_interface(void)
{
   U8 u8_i;

   // wValue = Alternate Setting
   // wIndex = Interface
   U16 wValue  = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
   U16 wIndex  = usb_format_usb_to_mcu_data(16, Usb_read_endpoint_data(EP_CONTROL, 16));
   Usb_ack_setup_received_free();

   // Get descriptor

   data_to_transfer = Usb_get_conf_desc_length();  //!< sizeof(usb_conf_desc);
   pbuffer          = Usb_get_conf_desc_pointer();

   //** Scan descriptor

   //* Find configuration selected
   if( usb_configuration_nb == 0 )
   {
      // No configuration selected then no interface enable
      Usb_enable_stall_handshake(EP_CONTROL);
      Usb_ack_setup_received_free();
      return;
   }
   u8_i = usb_configuration_nb;
   while( u8_i != 0 )
   {
      if( CONFIGURATION_DESCRIPTOR != ((S_usb_configuration_descriptor*)pbuffer)->bDescriptorType )
      {
         data_to_transfer -=  ((S_usb_configuration_descriptor*)pbuffer)->bLength;
         pbuffer =  (U8*)pbuffer + ((S_usb_configuration_descriptor*)pbuffer)->bLength;
         continue;
      }
      u8_i--;
      if( u8_i != 0 )
      {
         data_to_transfer -=  ((S_usb_configuration_descriptor*)pbuffer)->wTotalLength;
         pbuffer =  (U8*)pbuffer + ((S_usb_configuration_descriptor*)pbuffer)->wTotalLength;
      }
   }

   // Find interface selected
   if( wIndex >= ((S_usb_configuration_descriptor*)pbuffer)->bNumInterfaces )
   {
      // Interface number unknow
      Usb_enable_stall_handshake(EP_CONTROL);
      Usb_ack_setup_received_free();
      return;
   }
   while( 1 )
   {
      if( data_to_transfer <= ((S_usb_interface_descriptor*)pbuffer)->bLength )
      {
         // Interface unknow
         Usb_enable_stall_handshake(EP_CONTROL);
         Usb_ack_setup_received_free();
         return;
      }
      data_to_transfer -=  ((S_usb_interface_descriptor*)pbuffer)->bLength;
      pbuffer =  (U8*)pbuffer + ((S_usb_interface_descriptor*)pbuffer)->bLength;
      if( INTERFACE_DESCRIPTOR != ((S_usb_interface_descriptor*)pbuffer)->bDescriptorType )
         continue;
      if( wIndex != ((S_usb_interface_descriptor*)pbuffer)->bInterfaceNumber )
         continue;
      if( wValue != ((S_usb_interface_descriptor*)pbuffer)->bAlternateSetting )
         continue;
      usb_interface_status[wIndex] = wValue;
      break;
   }

   //* Find endpoints of interface and reset it
   while( 1 )
   {
      if( data_to_transfer <= ((S_usb_endpoint_descriptor*)pbuffer)->bLength )
         break;    // End of interface
      data_to_transfer -=  ((S_usb_endpoint_descriptor*)pbuffer)->bLength;
      pbuffer =  (U8*)pbuffer + ((S_usb_endpoint_descriptor*)pbuffer)->bLength;
      if( INTERFACE_DESCRIPTOR == ((S_usb_endpoint_descriptor*)pbuffer)->bDescriptorType )
         break;    // End of interface
      if( ENDPOINT_DESCRIPTOR == ((S_usb_endpoint_descriptor*)pbuffer)->bDescriptorType )
      {
         // Reset endpoint
         u8_i = ((S_usb_endpoint_descriptor*)pbuffer)->bEndpointAddress & (~MSK_EP_DIR);
         Usb_disable_stall_handshake(u8_i);
         Usb_reset_endpoint(u8_i);
         Usb_reset_data_toggle(u8_i);
      }
   }

   // send a ZLP for STATUS phase
   Usb_ack_control_in_ready_send();
   while (!Is_usb_control_in_ready());
}

//! This function returns the size and the pointer on a user information
//! structure
//!
Bool usb_user_get_descriptor(U8 type, U8 string)
{
  return FALSE;
}
//! This function is called by the standard USB read request function when
//! the USB request is not supported. This function returns TRUE when the
//! request is processed. This function returns FALSE if the request is not
//! supported. In this case, a STALL handshake will be automatically
//! sent by the standard USB read request function.
//!
Bool usb_user_read_request(U8 type, U8 request)
{
  switch (request)
  {
    case GET_LINE_CODING:
      cdc_get_line_coding();
      return TRUE;
      // No need to break here !

    case SET_LINE_CODING:
      cdc_set_line_coding();
      return TRUE;
      // No need to break here !

    case SET_CONTROL_LINE_STATE:
      cdc_set_control_line_state();
      return TRUE;
      // No need to break here !

    default:
      return FALSE;
      // No need to break here !
  }
}

//! @brief This function configures the endpoints of the device application.
//! This function is called when the set configuration request has been received.
//!
void usb_user_endpoint_init(U8 conf_nb)
{


  (void)Usb_configure_endpoint(TX_EP,
                         EP_ATTRIBUTES_1,
                         DIRECTION_IN,
                         EP_SIZE_1_FS,
                         DOUBLE_BANK);

  (void)Usb_configure_endpoint(RX_EP,
                         EP_ATTRIBUTES_2,
                         DIRECTION_OUT,
                         EP_SIZE_2_FS,
                         DOUBLE_BANK);

  (void)Usb_configure_endpoint(INT_EP,
                         EP_ATTRIBUTES_3,
                         DIRECTION_IN,
                         EP_SIZE_3,
                         SINGLE_BANK);
}

//! usb_write_ep_txpacket
//!
//!  This function writes the buffer pointed to by txbuf to the selected
//!  endpoint FIFO, using as few accesses as possible.
//!
//! @param ep           Number of the addressed endpoint
//! @param txbuf        Address of buffer to read
//! @param data_length  Number of bytes to write
//! @param ptxbuf       NULL or pointer to the buffer address to update
//!
//! @return             Number of non-written bytes
//!
//! @note The selected endpoint FIFO may be written in several steps by calling
//! usb_write_ep_txpacket several times.
//!
//! @warning Invoke Usb_reset_endpoint_fifo_access before this function when at
//! FIFO beginning whether or not the FIFO is to be written in several steps.
//!
//! @warning Do not mix calls to this function with calls to indexed macros.
//!
U32 usb_write_ep_txpacket(U8 ep, const void *txbuf, U32 data_length, const void **ptxbuf)
{
  // Use aggregated pointers to have several alignments available for a same address
  UnionVPtr   ep_fifo;
  UnionCPtr   txbuf_cur;
#if (!defined __OPTIMIZE_SIZE__) || !__OPTIMIZE_SIZE__  // Auto-generated when GCC's -Os command option is used
  StructCPtr  txbuf_end;
#else
  UnionCPtr   txbuf_end;
#endif  // !__OPTIMIZE_SIZE__

  // Initialize pointers for copy loops and limit the number of bytes to copy
  ep_fifo.u8ptr = pep_fifo[ep].u8ptr;
  txbuf_cur.u8ptr = txbuf;
  txbuf_end.u8ptr = txbuf_cur.u8ptr +
                    min(data_length, Usb_get_endpoint_size(ep) - Usb_byte_count(ep));
#if (!defined __OPTIMIZE_SIZE__) || !__OPTIMIZE_SIZE__  // Auto-generated when GCC's -Os command option is used
  txbuf_end.u16ptr = (U16 *)Align_down((U32)txbuf_end.u8ptr, sizeof(U16));
  txbuf_end.u32ptr = (U32 *)Align_down((U32)txbuf_end.u16ptr, sizeof(U32));
  txbuf_end.u64ptr = (U64 *)Align_down((U32)txbuf_end.u32ptr, sizeof(U64));

  // If all addresses are aligned the same way with respect to 16-bit boundaries
  if (Get_align((U32)txbuf_cur.u8ptr, sizeof(U16)) == Get_align((U32)ep_fifo.u8ptr, sizeof(U16)))
  {
    // If pointer to transmission buffer is not 16-bit aligned
    if (!Test_align((U32)txbuf_cur.u8ptr, sizeof(U16)))
    {
      // Copy 8-bit data to reach 16-bit alignment
      if (txbuf_cur.u8ptr < txbuf_end.u8ptr)
      {
        // 8-bit accesses to FIFO data registers do require pointer post-increment
        *ep_fifo.u8ptr++ = *txbuf_cur.u8ptr++;
      }
    }

    // If all addresses are aligned the same way with respect to 32-bit boundaries
    if (Get_align((U32)txbuf_cur.u16ptr, sizeof(U32)) == Get_align((U32)ep_fifo.u16ptr, sizeof(U32)))
    {
      // If pointer to transmission buffer is not 32-bit aligned
      if (!Test_align((U32)txbuf_cur.u16ptr, sizeof(U32)))
      {
        // Copy 16-bit data to reach 32-bit alignment
        if (txbuf_cur.u16ptr < txbuf_end.u16ptr)
        {
          // 16-bit accesses to FIFO data registers do require pointer post-increment
          *ep_fifo.u16ptr++ = *txbuf_cur.u16ptr++;
        }
      }

      // If pointer to transmission buffer is not 64-bit aligned
      if (!Test_align((U32)txbuf_cur.u32ptr, sizeof(U64)))
      {
        // Copy 32-bit data to reach 64-bit alignment
        if (txbuf_cur.u32ptr < txbuf_end.u32ptr)
        {
          // 32-bit accesses to FIFO data registers do not require pointer post-increment
          *ep_fifo.u32ptr = *txbuf_cur.u32ptr++;
        }
      }

      // Copy 64-bit-aligned data
      while (txbuf_cur.u64ptr < txbuf_end.u64ptr)
      {
        // 64-bit accesses to FIFO data registers do not require pointer post-increment
        *ep_fifo.u64ptr = *txbuf_cur.u64ptr++;
      }

      // Copy 32-bit-aligned data
      if (txbuf_cur.u32ptr < txbuf_end.u32ptr)
      {
        // 32-bit accesses to FIFO data registers do not require pointer post-increment
        *ep_fifo.u32ptr = *txbuf_cur.u32ptr++;
      }
    }

    // Copy remaining 16-bit data if some
    while (txbuf_cur.u16ptr < txbuf_end.u16ptr)
    {
      // 16-bit accesses to FIFO data registers do require pointer post-increment
      *ep_fifo.u16ptr++ = *txbuf_cur.u16ptr++;
    }
  }

#endif  // !__OPTIMIZE_SIZE__

  // Copy remaining 8-bit data if some
  while (txbuf_cur.u8ptr < txbuf_end.u8ptr)
  {
    // 8-bit accesses to FIFO data registers do require pointer post-increment
    *ep_fifo.u8ptr++ = *txbuf_cur.u8ptr++;
  }

  // Save current position in FIFO data register
  pep_fifo[ep].u8ptr = ep_fifo.u8ptr;

  // Return the updated buffer address and the number of non-copied bytes
  if (ptxbuf) *ptxbuf = txbuf_cur.u8ptr;
  return data_length - (txbuf_cur.u8ptr - (U8 *)txbuf);
}

//! usb_read_ep_rxpacket
//!
//!  This function reads the selected endpoint FIFO to the buffer pointed to by
//!  rxbuf, using as few accesses as possible.
//!
//! @param ep           Number of the addressed endpoint
//! @param rxbuf        Address of buffer to write
//! @param data_length  Number of bytes to read
//! @param prxbuf       NULL or pointer to the buffer address to update
//!
//! @return             Number of non-read bytes
//!
//! @note The selected endpoint FIFO may be read in several steps by calling
//! usb_read_ep_rxpacket several times.
//!
//! @warning Invoke Usb_reset_endpoint_fifo_access before this function when at
//! FIFO beginning whether or not the FIFO is to be read in several steps.
//!
//! @warning Do not mix calls to this function with calls to indexed macros.
//!
U32 usb_read_ep_rxpacket(U8 ep, void *rxbuf, U32 data_length, void **prxbuf)
{
  // Use aggregated pointers to have several alignments available for a same address
  UnionCVPtr  ep_fifo;
  UnionPtr    rxbuf_cur;
#if (!defined __OPTIMIZE_SIZE__) || !__OPTIMIZE_SIZE__  // Auto-generated when GCC's -Os command option is used
  StructCPtr  rxbuf_end;
#else
  StructCPtr  rxbuf_end;
#endif  // !__OPTIMIZE_SIZE__

  // Initialize pointers for copy loops and limit the number of bytes to copy
  ep_fifo.u8ptr = pep_fifo[ep].u8ptr;
  rxbuf_cur.u8ptr = rxbuf;
  rxbuf_end.u8ptr = rxbuf_cur.u8ptr + min(data_length, Usb_byte_count(ep));
#if (!defined __OPTIMIZE_SIZE__) || !__OPTIMIZE_SIZE__  // Auto-generated when GCC's -Os command option is used
  rxbuf_end.u16ptr = (U16 *)Align_down((U32)rxbuf_end.u8ptr, sizeof(U16));
  rxbuf_end.u32ptr = (U32 *)Align_down((U32)rxbuf_end.u16ptr, sizeof(U32));
  rxbuf_end.u64ptr = (U64 *)Align_down((U32)rxbuf_end.u32ptr, sizeof(U64));

  // If all addresses are aligned the same way with respect to 16-bit boundaries
  if (Get_align((U32)rxbuf_cur.u8ptr, sizeof(U16)) == Get_align((U32)ep_fifo.u8ptr, sizeof(U16)))
  {
    // If pointer to reception buffer is not 16-bit aligned
    if (!Test_align((U32)rxbuf_cur.u8ptr, sizeof(U16)))
    {
      // Copy 8-bit data to reach 16-bit alignment
      if (rxbuf_cur.u8ptr < rxbuf_end.u8ptr)
      {
        // 8-bit accesses to FIFO data registers do require pointer post-increment
        *rxbuf_cur.u8ptr++ = *ep_fifo.u8ptr++;
      }
    }

    // If all addresses are aligned the same way with respect to 32-bit boundaries
    if (Get_align((U32)rxbuf_cur.u16ptr, sizeof(U32)) == Get_align((U32)ep_fifo.u16ptr, sizeof(U32)))
    {
      // If pointer to reception buffer is not 32-bit aligned
      if (!Test_align((U32)rxbuf_cur.u16ptr, sizeof(U32)))
      {
        // Copy 16-bit data to reach 32-bit alignment
        if (rxbuf_cur.u16ptr < rxbuf_end.u16ptr)
        {
          // 16-bit accesses to FIFO data registers do require pointer post-increment
          *rxbuf_cur.u16ptr++ = *ep_fifo.u16ptr++;
        }
      }

      // If pointer to reception buffer is not 64-bit aligned
      if (!Test_align((U32)rxbuf_cur.u32ptr, sizeof(U64)))
      {
        // Copy 32-bit data to reach 64-bit alignment
        if (rxbuf_cur.u32ptr < rxbuf_end.u32ptr)
        {
          // 32-bit accesses to FIFO data registers do not require pointer post-increment
          *rxbuf_cur.u32ptr++ = *ep_fifo.u32ptr;
        }
      }

      // Copy 64-bit-aligned data
      while (rxbuf_cur.u64ptr < rxbuf_end.u64ptr)
      {
        // 64-bit accesses to FIFO data registers do not require pointer post-increment
        *rxbuf_cur.u64ptr++ = *ep_fifo.u64ptr;
      }

      // Copy 32-bit-aligned data
      if (rxbuf_cur.u32ptr < rxbuf_end.u32ptr) {
        // 32-bit accesses to FIFO data registers do not require pointer post-increment
        *rxbuf_cur.u32ptr++ = *ep_fifo.u32ptr;
      }
    }

    // Copy remaining 16-bit data if some
    while (rxbuf_cur.u16ptr < rxbuf_end.u16ptr) {
      // 16-bit accesses to FIFO data registers do require pointer post-increment
      *rxbuf_cur.u16ptr++ = *ep_fifo.u16ptr++;
    }
  }

#endif  // !__OPTIMIZE_SIZE__

  // Copy remaining 8-bit data if some
  while (rxbuf_cur.u8ptr < rxbuf_end.u8ptr) {
    // 8-bit accesses to FIFO data registers do require pointer post-increment
    *rxbuf_cur.u8ptr++ = *ep_fifo.u8ptr++;
  }

  // Save current position in FIFO data register
  pep_fifo[ep].u8ptr = (volatile U8 *)ep_fifo.u8ptr;

  // Return the updated buffer address and the number of non-copied bytes
  if (prxbuf) *prxbuf = rxbuf_cur.u8ptr;
  return data_length - (rxbuf_cur.u8ptr - (U8 *)rxbuf);
}

void cdc_get_line_coding(void)
{
   Usb_ack_setup_received_free();

   Usb_reset_endpoint_fifo_access(EP_CONTROL);
   Usb_write_endpoint_data(EP_CONTROL, 8, LSB0(line_coding.dwDTERate));
   Usb_write_endpoint_data(EP_CONTROL, 8, LSB1(line_coding.dwDTERate));
   Usb_write_endpoint_data(EP_CONTROL, 8, LSB2(line_coding.dwDTERate));
   Usb_write_endpoint_data(EP_CONTROL, 8, LSB3(line_coding.dwDTERate));
   Usb_write_endpoint_data(EP_CONTROL, 8, line_coding.bCharFormat);
   Usb_write_endpoint_data(EP_CONTROL, 8, line_coding.bParityType);
   Usb_write_endpoint_data(EP_CONTROL, 8, line_coding.bDataBits  );

   Usb_ack_control_in_ready_send();
   while (!Is_usb_control_in_ready());

   while(!Is_usb_control_out_received());
   Usb_ack_control_out_received_free();
}

void cdc_set_line_coding (void)
{
   Usb_ack_setup_received_free();

   while(!Is_usb_control_out_received());
   Usb_reset_endpoint_fifo_access(EP_CONTROL);

   LSB0(line_coding.dwDTERate) = Usb_read_endpoint_data(EP_CONTROL, 8);
   LSB1(line_coding.dwDTERate) = Usb_read_endpoint_data(EP_CONTROL, 8);
   LSB2(line_coding.dwDTERate) = Usb_read_endpoint_data(EP_CONTROL, 8);
   LSB3(line_coding.dwDTERate) = Usb_read_endpoint_data(EP_CONTROL, 8);
   line_coding.bCharFormat = Usb_read_endpoint_data(EP_CONTROL, 8);
   line_coding.bParityType = Usb_read_endpoint_data(EP_CONTROL, 8);
   line_coding.bDataBits = Usb_read_endpoint_data(EP_CONTROL, 8);
   Usb_ack_control_out_received_free();

   Usb_ack_control_in_ready_send();
   while (!Is_usb_control_in_ready());

   // Set the baudrate of the USART
   {
      static usart_options_t dbg_usart_options;
      U32 stopbits, parity;

      if     ( line_coding.bCharFormat==0 )   stopbits = USART_1_STOPBIT;
      else if( line_coding.bCharFormat==1 )   stopbits = USART_1_5_STOPBITS;
      else                                    stopbits = USART_2_STOPBITS;

      if     ( line_coding.bParityType==0 )   parity = USART_NO_PARITY;
      else if( line_coding.bParityType==1 )   parity = USART_ODD_PARITY;
      else if( line_coding.bParityType==2 )   parity = USART_EVEN_PARITY;
      else if( line_coding.bParityType==3 )   parity = USART_MARK_PARITY;
      else                                    parity = USART_SPACE_PARITY;

      // Options for debug USART.
      dbg_usart_options.baudrate    = line_coding.dwDTERate;
      dbg_usart_options.charlength  = line_coding.bDataBits;
      dbg_usart_options.paritytype  = parity;
      dbg_usart_options.stopbits    = stopbits;
      dbg_usart_options.channelmode = USART_NORMAL_CHMODE;
   }
}

void cdc_set_control_line_state (void)
{
   Usb_ack_setup_received_free();
   Usb_ack_control_in_ready_send();
   while (!Is_usb_control_in_ready());
}
/*************** USB VIRTUAL UART ************************/

Bool UsbCdcTestHit(void)
{
   if( Is_usb_out_received(RX_EP))  {
      if( 0!=Usb_byte_count(RX_EP) ) {
         if( b_rx_new ) {
            Usb_reset_endpoint_fifo_access(RX_EP);
            b_rx_new = FALSE;
         }
         return TRUE;
      }
      Usb_ack_out_received_free(RX_EP);
      b_rx_new = TRUE;
   }
   return FALSE;
}

int UsbCdcReadChar( int *c)
{
   // No error; if we really did receive a char, read it and return SUCCESS.
  if (UsbCdcTestHit()) {
    *c = Usb_read_endpoint_data(RX_EP, 8);
    if( 0==Usb_byte_count(RX_EP) ) {
        Usb_ack_out_received_free(RX_EP);
        b_rx_new = TRUE;
    }
    return 1;
  } else
      return 0;
}

// Functions that manage characters output through USB
//
Bool UsbCdcTxReady(void)
{
  if( Is_usb_write_enabled(TX_EP) ) {
     if( b_tx_new ) {
       Usb_reset_endpoint_fifo_access(TX_EP);
       b_tx_new = FALSE;
     }
      return TRUE;
  }
  return FALSE;
}

int UsbCdcSendChar(int data_to_send)
{
  while( !UsbCdcTxReady() ); // Wait Endpoint ready
  Usb_write_endpoint_data(TX_EP, 8, data_to_send);
  if( !Is_usb_write_enabled(TX_EP) ) { // If Endpoint full -> flush
    Usb_ack_in_ready_send(TX_EP);
    b_tx_new = TRUE;
  }
  return data_to_send;
}

void UsbCdcFlush (void)
{
  if( 0 != Usb_nb_busy_bank(TX_EP) )
    return; // Don't flush because a previous flush is on-going and all bank don't be busy
  Usb_ack_in_ready_send(TX_EP);
  b_tx_new = TRUE;
}

#endif
