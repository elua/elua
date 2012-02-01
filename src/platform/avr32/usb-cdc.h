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

#ifndef _USB_CDC_H_
#define _USB_CDC_H_

#include <avr32/io.h>
#include "compiler.h"


#define USB_CONFIG_ATTRIBUTES_RESERVED        0x80
#define USB_CONFIG_BUSPOWERED                 (USB_CONFIG_ATTRIBUTES_RESERVED | 0x00)
#define USB_CONFIG_SELFPOWERED                (USB_CONFIG_ATTRIBUTES_RESERVED | 0x40)
#define BUS_POWERED                           0
#define SELF_POWERED                          1
#define MSK_EP_DIR                            0x80
#define MSK_EP_NBR                            0x0F

#define DIRECTION_OUT                         AVR32_USBB_UECFG0_EPDIR_OUT
#define DIRECTION_IN                          AVR32_USBB_UECFG0_EPDIR_IN
#define SINGLE_BANK                           AVR32_USBB_UECFG0_EPBK_SINGLE
#define DOUBLE_BANK                           AVR32_USBB_UECFG0_EPBK_DOUBLE
#define TRIPLE_BANK                           AVR32_USBB_UECFG0_EPBK_TRIPLE


#define EVT_USB_POWERED               1         // USB plugged
#define EVT_USB_UNPOWERED             2         // USB unplugged
#define EVT_USB_DEVICE_FUNCTION       3         // USB in device
#define EVT_USB_HOST_FUNCTION         4         // USB in host
#define EVT_USB_SUSPEND               5         // USB suspend
#define EVT_USB_WAKE_UP               6         // USB wake-up
#define EVT_USB_RESUME                7         // USB resume
#define EVT_USB_RESET                 8         // USB reset
#define EVT_HOST_SOF                  9         // Host start-of-frame sent
#define EVT_HOST_HWUP                 10        // Host wake-up detected
#define EVT_HOST_DISCONNECTION        11        // The target device is disconnected

#define NB_MS_BEFORE_FLUSH            1

#define AVR32_USBB_usbcon             (AVR32_USBB.usbcon)
#define AVR32_USBB_udintclr           (AVR32_USBB.udintclr)
#define AVR32_USBB_usbsta             (AVR32_USBB.usbsta)
#define AVR32_USBB_usbstaclr          (AVR32_USBB.usbstaclr)
#define AVR32_USBB_udint              (AVR32_USBB.udint)
#define AVR32_USBB_udinte             (AVR32_USBB.udinte)
#define AVR32_USBB_udinteclr          (AVR32_USBB.udinteclr)
#define AVR32_USBB_udinteset          (AVR32_USBB.udinteset)
#define AVR32_USBB_udcon              (AVR32_USBB.udcon)
#define AVR32_USBB_uerst              (AVR32_USBB.uerst)
#define AVR32_USBB_uecfgx(x)          ((&AVR32_USBB.uecfg0)[(x)])
#define AVR32_USBB_uestax(x)          ((&AVR32_USBB.uesta0)[(x)])
#define AVR32_USBB_ueconxset(x)       ((&AVR32_USBB.uecon0set)[(x)])
#define AVR32_USBB_uestaxclr(x)       ((&AVR32_USBB.uesta0clr)[(x)])
#define AVR32_USBB_ueconxclr(x)       ((&AVR32_USBB.uecon0clr)[(x)])
#define AVR32_USBB_ueconx(x)          ((&AVR32_USBB.uecon0)[(x)])

#define Usb_disable_id_pin()          (Clr_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_UIDE_MASK))
#define Usb_ack_control_out_received_free()       (AVR32_USBB_uestaxclr(EP_CONTROL) = AVR32_USBB_UESTA0CLR_RXOUTIC_MASK)

#define AVR32_USBB_FIFOX_DATA(x, scale)  (((volatile TPASTE2(U, scale) (*)[0x10000 / ((scale) / 8)])AVR32_USBB_SLAVE)[(x)])

#define Usb_reset_endpoint_fifo_access(ep) \
          (pep_fifo[(ep)].u64ptr = Usb_get_endpoint_fifo_access(ep, 64))

#define Usb_read_endpoint_data(ep, scale) \
          (*pep_fifo[(ep)].TPASTE3(u, scale, ptr)\
           TPASTE3(Pep_fifo_access_, scale, _post_inc)())

#define Usb_write_endpoint_data(ep, scale, data) \
          (*pep_fifo[(ep)].TPASTE3(u, scale, ptr)\
           TPASTE3(Pep_fifo_access_, scale, _post_inc)() = (data))

#define Usb_read_endpoint_indexed_data(ep, scale, index) \
          (AVR32_USBB_FIFOX_DATA(ep, scale)[(index)])

#define Usb_write_endpoint_indexed_data(ep, scale, index, data) \
          (AVR32_USBB_FIFOX_DATA(ep, scale)[(index)] = (data))
#define Usb_enable_endpoint_int_dis_hdma_req(ep)      (AVR32_USBB_ueconxset(ep) = AVR32_USBB_UECONXSET_EPDISHDMAS_MASK)
#define Usb_disable_endpoint_int_dis_hdma_req(ep)     (AVR32_USBB_ueconxclr(ep) = AVR32_USBB_UECONXCLR_EPDISHDMAC_MASK)
#define Is_usb_endpoint_int_dis_hdma_req_enabled(ep)  (Tst_bits(AVR32_USBB_ueconx(ep), AVR32_USBB_UECON0_EPDISHDMA_MASK))
#define Usb_raise_endpoint_dma_interrupt(epdma)       (AVR32_USBB_udintset = AVR32_USBB_UDINTSET_DMA1INTS_MASK << ((epdma) - 1))
#define Is_usb_endpoint_dma_interrupt(epdma)          (Tst_bits(AVR32_USBB_udint, AVR32_USBB_UDINT_DMA1INT_MASK << ((epdma) - 1)))
#define Usb_enable_endpoint_dma_interrupt(epdma)      (AVR32_USBB_udinteset = AVR32_USBB_UDINTESET_DMA1INTES_MASK << ((epdma) - 1))
#define Usb_disable_endpoint_dma_interrupt(epdma)     (AVR32_USBB_udinteclr = AVR32_USBB_UDINTECLR_DMA1INTEC_MASK << ((epdma) - 1))
#define Is_usb_endpoint_dma_interrupt_enabled(epdma)  (Tst_bits(AVR32_USBB_udinte, AVR32_USBB_UDINTE_DMA1INTE_MASK << ((epdma) - 1)))
#define Usb_ack_setup_received_free()             (AVR32_USBB_uestaxclr(EP_CONTROL) = AVR32_USBB_UESTA0CLR_RXSTPIC_MASK)
#define Usb_send_in(ep)                           (Usb_ack_fifocon(ep))
#define Is_usb_out_received(ep)                   (Tst_bits(AVR32_USBB_uestax(ep), AVR32_USBB_UESTA0_RXOUTI_MASK))
#define Is_usb_out_received_interrupt_enabled(ep) (Tst_bits(AVR32_USBB_ueconx(ep), AVR32_USBB_UECON0_RXOUTE_MASK))
#define Usb_ack_fifocon(ep)                       (AVR32_USBB_ueconxclr(ep) = AVR32_USBB_UECON0CLR_FIFOCONC_MASK)
#define Usb_ack_out_received_free(ep)             (Usb_ack_out_received(ep), Usb_free_out(ep))
#define Usb_ack_out_received(ep)                  (AVR32_USBB_uestaxclr(ep) = AVR32_USBB_UESTA0CLR_RXOUTIC_MASK)
#define Is_usb_nak_out(ep)                        (Tst_bits(AVR32_USBB_uestax(ep), AVR32_USBB_UESTA0_NAKOUTI_MASK))
#define Usb_enable_sof_interrupt()                (AVR32_USBB_udinteset = AVR32_USBB_UDINTESET_SOFES_MASK)
#define Get_desc_ep_nbr(ep_addr)                  (Rd_bitfield(ep_addr, MSK_EP_NBR))

#define Is_usb_vbus_high()                        (Tst_bits(AVR32_USBB_usbsta, AVR32_USBB_USBSTA_VBUS_MASK))
#define Usb_detach()                              (Set_bits(AVR32_USBB_udcon, AVR32_USBB_UDCON_DETACH_MASK))
#define Usb_ack_sof()                             (AVR32_USBB_udintclr = AVR32_USBB_UDINTCLR_SOFC_MASK)

#define Usb_send_event(x)             (Set_bits(g_usb_event, 1 << (x)))
#define Usb_ack_event(x)              (Clr_bits(g_usb_event, 1 << (x)))
#define Is_usb_event(x)               (Tst_bits(g_usb_event, 1 << (x)))
#define Usb_clear_all_event()         (g_usb_event = 0x0000)
#define Is_device_enumerated()        (usb_configuration_nb != 0)
#define Is_host_emergency_exit()      (Is_usb_device() || Is_usb_event(EVT_HOST_DISCONNECTION) || Is_usb_event(EVT_USB_DEVICE_FUNCTION))

#define Is_usb_enabled()              (Tst_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_USBE_MASK))
#define Is_usb_clock_frozen()         (Tst_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_FRZCLK_MASK))
#define Usb_enable()                  (Set_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_USBE_MASK))
#define Usb_disable()                 (Clr_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_USBE_MASK))
#define Usb_attach()                  (Clr_bits(AVR32_USBB_udcon, AVR32_USBB_UDCON_DETACH_MASK))
#define Usb_enable_otg_pad()          (Set_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_OTGPADE_MASK))
#define Usb_disable_otg_pad()         (Clr_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_OTGPADE_MASK))
#define Usb_unfreeze_clock()          (Clr_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_FRZCLK_MASK))
#define Usb_enable_vbus_interrupt()   (Set_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_VBUSTE_MASK))
#define Usb_ack_suspend()             (AVR32_USBB_udintclr = AVR32_USBB_UDINTCLR_SUSPC_MASK)
#define Is_usb_vbus_transition()           (Tst_bits(AVR32_USBB_usbsta, AVR32_USBB_USBSTA_VBUSTI_MASK))
#define Is_usb_vbus_interrupt_enabled()    (Tst_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_VBUSTE_MASK))
#define Usb_ack_vbus_transition()          (AVR32_USBB_usbstaclr = AVR32_USBB_USBSTACLR_VBUSTIC_MASK)
#define Is_usb_sof_interrupt_enabled()     (Tst_bits(AVR32_USBB_udinte, AVR32_USBB_UDINTE_SOFE_MASK))
#define Is_usb_sof()                       (Tst_bits(AVR32_USBB_udint, AVR32_USBB_UDINT_SOF_MASK))
#define Usb_force_device_mode()            (Set_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_UIMOD_MASK), Usb_disable_id_pin())
#define Is_usb_reset_interrupt_enabled()   (Tst_bits(AVR32_USBB_udinte, AVR32_USBB_UDINTE_EORSTE_MASK))
#define Is_usb_reset()                     (Tst_bits(AVR32_USBB_udint, AVR32_USBB_UDINT_EORST_MASK))
#define Is_usb_reset_interrupt_enabled()   (Tst_bits(AVR32_USBB_udinte, AVR32_USBB_UDINTE_EORSTE_MASK))
#define Is_usb_suspend()                   (Tst_bits(AVR32_USBB_udint, AVR32_USBB_UDINT_SUSP_MASK))
#define Usb_disable_resume_interrupt()     (AVR32_USBB_udinteclr = AVR32_USBB_UDINTECLR_EORSMEC_MASK)
#define Usb_enable_suspend_interrupt()     (AVR32_USBB_udinteset = AVR32_USBB_UDINTESET_SUSPES_MASK)
#define Usb_enable_reset_interrupt()       (AVR32_USBB_udinteset = AVR32_USBB_UDINTESET_EORSTES_MASK)
#define Usb_force_full_speed_mode()        (Wr_bitfield(AVR32_USBB_udcon, AVR32_USBB_UDCON_SPDCONF_MASK, 3))
#define Is_usb_id_device()                 (Tst_bits(AVR32_USBB_usbsta, AVR32_USBB_USBSTA_ID_MASK))
#define Usb_ack_id_transition()       (AVR32_USBB_usbstaclr = AVR32_USBB_USBSTACLR_IDTIC_MASK)
#define Usb_raise_id_transition()     (AVR32_USBB_usbstaset = AVR32_USBB_USBSTASET_IDTIS_MASK)
#define Is_usb_id_transition()        (Tst_bits(AVR32_USBB_usbsta, AVR32_USBB_USBSTA_IDTI_MASK))
#define Usb_enable_id_interrupt()     (Set_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_IDTE_MASK))
#define Usb_disable_id_interrupt()    (Clr_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_IDTE_MASK))
#define Is_usb_id_interrupt_enabled() (Tst_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_IDTE_MASK))
#define Usb_format_endpoint_size(size)     (32 - clz(((U32)min(max(size, 8), 1024) << 1) - 1) - 1 - 3)
#define Is_usb_endpoint_configured(ep)     (Tst_bits(AVR32_USBB_uestax(ep), AVR32_USBB_UESTA0_CFGOK_MASK))
#define Usb_enable_endpoint(ep)            (Set_bits(AVR32_USBB_uerst, AVR32_USBB_UERST_EPEN0_MASK << (ep)))
#define Usb_allocate_memory(ep)            (Set_bits(AVR32_USBB_uecfgx(ep), AVR32_USBB_UECFG0_ALLOC_MASK))
#define Usb_get_endpoint_fifo_access(ep, scale)     (AVR32_USBB_FIFOX_DATA(ep, scale))
#define Usb_byte_count(ep)                 (Rd_bitfield(AVR32_USBB_uestax(ep), AVR32_USBB_UESTA0_BYCT_MASK))
#define Usb_free_out(ep)                   (Usb_ack_fifocon(ep))
#define Usb_get_dev_desc_length()          (sizeof(usb_dev_desc))
#define Usb_configure_address(addr)        (Wr_bitfield(AVR32_USBB_udcon, AVR32_USBB_UDCON_UADD_MASK, addr))
#define Is_usb_control_in_ready()          (Tst_bits(AVR32_USBB_uestax(EP_CONTROL), AVR32_USBB_UESTA0_TXINI_MASK))
#define Usb_ack_control_in_ready_send()           (AVR32_USBB_uestaxclr(EP_CONTROL) = AVR32_USBB_UESTA0CLR_TXINIC_MASK)
#define Usb_ack_nak_out(ep)                       (AVR32_USBB_uestaxclr(ep) = AVR32_USBB_UESTA0CLR_NAKOUTIC_MASK)
#define Usb_get_endpoint_size(ep)                 (8 << Rd_bitfield(AVR32_USBB_uecfgx(ep), AVR32_USBB_UECFG0_EPSIZE_MASK))
#define Is_usb_control_out_received()             (Tst_bits(AVR32_USBB_uestax(EP_CONTROL), AVR32_USBB_UESTA0_RXOUTI_MASK))
#define Usb_enable_address()                      (Set_bits(AVR32_USBB_udcon, AVR32_USBB_UDCON_ADDEN_MASK))
#define Usb_reset_data_toggle(ep)                 (AVR32_USBB_ueconxset(ep) = AVR32_USBB_UECON0SET_RSTDTS_MASK)
#define Is_usb_endpoint_stall_requested(ep)       (Tst_bits(AVR32_USBB_ueconx(ep), AVR32_USBB_UECON0_STALLRQ_MASK))

#define Usb_format_mcu_to_usb_data(width, data) (TPASTE2(Swap, width)(data))
#define Usb_format_usb_to_mcu_data(width, data) (TPASTE2(Swap, width)(data))
#define usb_format_mcu_to_usb_data(width, data) (TPASTE2(swap, width)(data))
#define usb_format_usb_to_mcu_data(width, data) (TPASTE2(swap, width)(data))


  //! configures selected endpoint in one step
#define Usb_configure_endpoint(ep, type, dir, size, bank) \
(\
  Usb_enable_endpoint(ep),\
  Wr_bits(AVR32_USBB_uecfgx(ep), AVR32_USBB_UECFG0_EPTYPE_MASK |\
                                 AVR32_USBB_UECFG0_EPDIR_MASK  |\
                                 AVR32_USBB_UECFG0_EPSIZE_MASK |\
                                 AVR32_USBB_UECFG0_EPBK_MASK,   \
          (((U32)(type) << AVR32_USBB_UECFG0_EPTYPE_OFFSET) & AVR32_USBB_UECFG0_EPTYPE_MASK) |\
          (((U32)(dir ) << AVR32_USBB_UECFG0_EPDIR_OFFSET ) & AVR32_USBB_UECFG0_EPDIR_MASK ) |\
          ( (U32)Usb_format_endpoint_size(size) << AVR32_USBB_UECFG0_EPSIZE_OFFSET         ) |\
          (((U32)(bank) << AVR32_USBB_UECFG0_EPBK_OFFSET  ) & AVR32_USBB_UECFG0_EPBK_MASK  )),\
  Usb_allocate_memory(ep),\
\
  Is_usb_endpoint_configured(ep)\
)

#define Usb_reset_endpoint_fifo_access(ep)    (pep_fifo[(ep)].u64ptr = Usb_get_endpoint_fifo_access(ep, 64))


#define Usb_wake_up_action()
#define Usb_resume_action()
#define Usb_suspend_action()
#define Usb_reset_action()
#define Usb_vbus_on_action()
#define Usb_vbus_off_action()
#define Usb_set_configuration_action()            Usb_enable_sof_interrupt()

#define Usb_reset_endpoint(ep)                    (Set_bits(AVR32_USBB_uerst, AVR32_USBB_UERST_EPRST0_MASK << (ep)),\
                                                   Clr_bits(AVR32_USBB_uerst, AVR32_USBB_UERST_EPRST0_MASK << (ep)))
#define Is_usb_vbus_transition()                  (Tst_bits(AVR32_USBB_usbsta, AVR32_USBB_USBSTA_VBUSTI_MASK))
#define Is_usb_resume_interrupt_enabled()         (Tst_bits(AVR32_USBB_udinte, AVR32_USBB_UDINTE_EORSME_MASK))
#define Is_usb_wake_up()                          (Tst_bits(AVR32_USBB_udint, AVR32_USBB_UDINT_WAKEUP_MASK))
#define Is_usb_sof()                              (Tst_bits(AVR32_USBB_udint, AVR32_USBB_UDINT_SOF_MASK))
#define Is_usb_suspend_interrupt_enabled()        (Tst_bits(AVR32_USBB_udinte, AVR32_USBB_UDINTE_SUSPE_MASK))
#define Is_usb_endpoint_enabled(ep)               (Tst_bits(AVR32_USBB_uerst, AVR32_USBB_UERST_EPEN0_MASK << (ep)))
#define Usb_ack_wake_up()                         (AVR32_USBB_udintclr = AVR32_USBB_UDINTCLR_WAKEUPC_MASK)
#define Is_usb_wake_up_interrupt_enabled()        (Tst_bits(AVR32_USBB_udinte, AVR32_USBB_UDINTE_WAKEUPE_MASK))
#define Usb_enable_wake_up_interrupt()            (AVR32_USBB_udinteset = AVR32_USBB_UDINTESET_WAKEUPES_MASK)
#define Usb_disable_wake_up_interrupt()           (AVR32_USBB_udinteclr = AVR32_USBB_UDINTECLR_WAKEUPEC_MASK)
#define Usb_ack_resume()                          (AVR32_USBB_udintclr = AVR32_USBB_UDINTCLR_EORSMC_MASK)
#define Usb_freeze_clock()                        (Set_bits(AVR32_USBB_usbcon, AVR32_USBB_USBCON_FRZCLK_MASK))
#define Usb_ack_reset()                           (AVR32_USBB_udintclr = AVR32_USBB_UDINTCLR_EORSTC_MASK)
#define Is_usb_setup_received()                   (Tst_bits(AVR32_USBB_uestax(EP_CONTROL), AVR32_USBB_UESTA0_RXSTPI_MASK))
#define Usb_disable_stall_handshake(ep)           (AVR32_USBB_ueconxclr(ep) = AVR32_USBB_UECON0CLR_STALLRQC_MASK)

#define Is_usb_resume()                           (Tst_bits(AVR32_USBB_udint, AVR32_USBB_UDINT_EORSM_MASK))

#define Usb_enable_stall_handshake(ep)            (AVR32_USBB_ueconxset(ep) = AVR32_USBB_UECON0SET_STALLRQS_MASK)
#define Is_usb_write_enabled(ep)                  (Tst_bits(AVR32_USBB_uestax(ep), AVR32_USBB_UESTA0_RWALL_MASK))
#define Usb_nb_busy_bank(ep)                      (Rd_bitfield(AVR32_USBB_uestax(ep), AVR32_USBB_UESTA0_NBUSYBK_MASK))
#define Usb_ack_in_ready_send(ep)                 (Usb_ack_in_ready(ep), Usb_send_in(ep))
#define Usb_ack_in_ready(ep)                      (AVR32_USBB_uestaxclr(ep) = AVR32_USBB_UESTA0CLR_TXINIC_MASK)


#define AVR32_USBB_IRQ                        544

#define USB_SERIALPORT                        0xE0

#define UART_USB_SUCCESS                      0 //!< Successful completion.
#define UART_USB_FAILURE                      -1 //!< Failure because of some unspecified reason.
#define UART_USB_RX_EMPTY                     3 //!< Nothing was received.

#define USART_1_STOPBIT                       AVR32_USART_MR_NBSTOP_1   //!< Use 1 stop bit.
#define USART_1_5_STOPBITS                    AVR32_USART_MR_NBSTOP_1_5 //!< Use 1.5 stop bits.
#define USART_2_STOPBITS                      AVR32_USART_MR_NBSTOP_2   //!< Use 2 stop bits (for more, just give the number of bits).
#define USART_NORMAL_CHMODE                   AVR32_USART_MR_CHMODE_NORMAL      //!< Normal communication.
#define USART_AUTO_ECHO                       AVR32_USART_MR_CHMODE_ECHO        //!< Echo data.
#define USART_LOCAL_LOOPBACK                  AVR32_USART_MR_CHMODE_LOCAL_LOOP  //!< Local loopback.
#define USART_REMOTE_LOOPBACK                 AVR32_USART_MR_CHMODE_REMOTE_LOOP //!< Remote loopback.
#define USART_EVEN_PARITY                     AVR32_USART_MR_PAR_EVEN   //!< Use even parity on character transmission.
#define USART_ODD_PARITY                      AVR32_USART_MR_PAR_ODD    //!< Use odd parity on character transmission.
#define USART_SPACE_PARITY                    AVR32_USART_MR_PAR_SPACE  //!< Use a space as parity bit.
#define USART_MARK_PARITY                     AVR32_USART_MR_PAR_MARK   //!< Use a mark as parity bit.
#define USART_NO_PARITY                       AVR32_USART_MR_PAR_NONE   //!< Don't use a parity bit.
#define USART_MULTIDROP_PARITY                AVR32_USART_MR_PAR_MULTI  //!< Parity bit is used to flag address characters.

#define EP_CONTROL                            0
#define MAX_PEP_NB                            AVR32_USBB_EPT_NUM
#define TYPE_BULK                             AVR32_USBB_UECFG0_EPTYPE_BULK
#define TYPE_INTERRUPT                        AVR32_USBB_UECFG0_EPTYPE_INTERRUPT

#define ATMEL_VID                             0x03EB
#define VENDOR_ID                             ATMEL_VID           //! Atmel vendor ID
#define USB_SPECIFICATION                     0x0200
#define CDC_COMM_CLASS                        0x02
#define CDC_COMM_DEVICE_CLASS                 0x02
#define CDC_DATA_CLASS                        0x0A

#define SET_LINE_CODING                       0x20
#define GET_LINE_CODING                       0x21
#define SET_CONTROL_LINE_STATE                0x22
#define SEND_BREAK                            0x23
#define SEND_ENCAPSULATED_COMMAND             0x00
#define GET_ENCAPSULATED_COMMAND              0x01

#define Pep_fifo_access_64_post_inc()
#define Pep_fifo_access_32_post_inc()
#define Pep_fifo_access_16_post_inc()         ++
#define Pep_fifo_access_8_post_inc()          ++

#define TX_EP                                 0x01
#define RX_EP                                 0x02
#define INT_EP                                0x03
#define GET_STATUS                            0x00
#define GET_DEVICE                            0x01
#define CLEAR_FEATURE                         0x01
#define GET_STRING                            0x03
#define SET_FEATURE                           0x03
#define SET_ADDRESS                           0x05
#define GET_DESCRIPTOR                        0x06
#define SET_DESCRIPTOR                        0x07
#define GET_CONFIGURATION                     0x08
#define SET_CONFIGURATION                     0x09
#define GET_INTERFACE                         0x0A
#define SET_INTERFACE                         0x0B
#define SYNCH_FRAME                           0x0C

#define GET_DEVICE_DESCRIPTOR                    1
#define GET_CONFIGURATION_DESCRIPTOR             4

#define REQUEST_DEVICE_STATUS                 0x80
#define REQUEST_INTERFACE_STATUS              0x81
#define REQUEST_ENDPOINT_STATUS               0x82
#define DEVICE_TYPE                           0x00
#define INTERFACE_TYPE                        0x01
#define ENDPOINT_TYPE                         0x02

                  // Descriptor Types
#define DEVICE_DESCRIPTOR                     0x01
#define CONFIGURATION_DESCRIPTOR              0x02
#define STRING_DESCRIPTOR                     0x03
#define INTERFACE_DESCRIPTOR                  0x04
#define ENDPOINT_DESCRIPTOR                   0x05
#define DEVICE_QUALIFIER_DESCRIPTOR           0x06
#define OTHER_SPEED_CONFIGURATION_DESCRIPTOR  0x07

                  // Standard Features
#define FEATURE_DEVICE_REMOTE_WAKEUP          0x01
#define FEATURE_ENDPOINT_HALT                 0x00
#define FEATURE_TEST_MODE                     0x02

       
/* usb_descriptors.h */

//_____ M A C R O S ________________________________________________________

#define Usb_unicode(c)                    (Usb_format_mcu_to_usb_data(16, (U16)(c)))
#define Usb_get_dev_desc_pointer()        (&(usb_dev_desc.bLength))
#define Usb_get_dev_desc_length()         (sizeof(usb_dev_desc))
#define Usb_get_conf_desc_pointer()       (&(usb_conf_desc_fs.cfg.bLength))
#define Usb_get_conf_desc_length()        (sizeof(usb_conf_desc_fs))
#define Usb_get_conf_desc_hs_pointer()    (&(usb_conf_desc_hs.cfg.bLength))
#define Usb_get_conf_desc_hs_length()     (sizeof(usb_conf_desc_hs))
#define Usb_get_conf_desc_fs_pointer()    (&(usb_conf_desc_fs.cfg.bLength))
#define Usb_get_conf_desc_fs_length()     (sizeof(usb_conf_desc_fs))
#define Usb_get_qualifier_desc_pointer()  (&(usb_qualifier_desc.bLength))
#define Usb_get_qualifier_desc_length()   (sizeof(usb_qualifier_desc))


//_____ U S B    D E F I N E S _____________________________________________

            // USB Device descriptor
#define USB_SPECIFICATION     0x0200
#define DEVICE_CLASS          CDC_COMM_DEVICE_CLASS
#define DEVICE_SUB_CLASS      0                   //! Each configuration has its own subclass
#define DEVICE_PROTOCOL       0                   //! Each configuration has its own protocol
#define EP_CONTROL_LENGTH     64
#define VENDOR_ID             ATMEL_VID           //! Atmel vendor ID
#define CDC_EXAMPLE_PID       0x2307
#define PRODUCT_ID            CDC_EXAMPLE_PID
#define RELEASE_NUMBER        0x1000
#define MAN_INDEX             0x01
#define PROD_INDEX            0x02
#define SN_INDEX              0x03
#define NB_CONFIGURATION      1

            // CONFIGURATION
#define NB_INTERFACE       2     //! The number of interfaces for this configuration
#define CONF_NB            1     //! Number of this configuration
#define CONF_INDEX         0
#define CONF_ATTRIBUTES    USB_CONFIG_BUSPOWERED
#define MAX_POWER          200    // 400 mA

            // Interface 0 descriptor
#define INTERFACE_NB_0              0                  //! The number of this interface
#define ALTERNATE_0                 0                  //! The alt setting nb of this interface
#define NB_ENDPOINT_0               1                  //! The number of endpoints this interface has
#define INTERFACE_CLASS_0           CDC_COMM_CLASS     //! CDC ACR Com Class
#define INTERFACE_SUB_CLASS_0       0x02
#define INTERFACE_PROTOCOL_0        0x01
#define INTERFACE_INDEX_0           0

            // Interface 1 descriptor
#define INTERFACE_NB_1              1                  //! The number of this interface
#define ALTERNATE_1                 0                  //! The alt setting nb of this interface
#define NB_ENDPOINT_1               2                  //! The number of endpoints this interface has
#define INTERFACE_CLASS_1           CDC_DATA_CLASS     //! CDC ACR Data Class
#define INTERFACE_SUB_CLASS_1       0
#define INTERFACE_PROTOCOL_1        0
#define INTERFACE_INDEX_1           0

             // USB Endpoint 3 descriptor
             // Interrupt IN
#define LANG_ID               0x00
#define ENDPOINT_NB_3       ( INT_EP | MSK_EP_DIR )
#define EP_ATTRIBUTES_3     TYPE_INTERRUPT
#define EP_IN_LENGTH_3      0x20
#define EP_SIZE_3           EP_IN_LENGTH_3
#define EP_INTERVAL_3       0xFF        //! Interrupt polling interval from host

             // USB Endpoint 1 descriptor
             // Bulk IN
#define ENDPOINT_NB_1       ( TX_EP | MSK_EP_DIR )
#define EP_ATTRIBUTES_1     TYPE_BULK
#define EP_IN_LENGTH_1_FS   0x40
#define EP_SIZE_1_FS        EP_IN_LENGTH_1_FS
#define EP_INTERVAL_1       0x00          //! Interrupt polling interval from host

             // USB Endpoint 2 descriptor
             // Bulk OUT
#define ENDPOINT_NB_2       RX_EP
#define EP_ATTRIBUTES_2     TYPE_BULK
#define EP_IN_LENGTH_2_FS   0x40
#define EP_SIZE_2_FS        EP_IN_LENGTH_2_FS
#define EP_INTERVAL_2       0x00          //! Interrupt polling interval from host


#define DEVICE_STATUS         SELF_POWERED
#define INTERFACE_STATUS      0x00 // TBD

#define LANG_ID               0x00

#define USB_MN_LENGTH         5
#define USB_MANUFACTURER_NAME \
{\
  Usb_unicode('A'),\
  Usb_unicode('T'),\
  Usb_unicode('M'),\
  Usb_unicode('E'),\
  Usb_unicode('L') \
}

#define USB_PN_LENGTH         13
#define USB_PRODUCT_NAME \
{\
  Usb_unicode('A'),\
  Usb_unicode('V'),\
  Usb_unicode('R'),\
  Usb_unicode('3'),\
  Usb_unicode('2'),\
  Usb_unicode(' '),\
  Usb_unicode('U'),\
  Usb_unicode('C'),\
  Usb_unicode('3'),\
  Usb_unicode(' '),\
  Usb_unicode('C'),\
  Usb_unicode('D'),\
  Usb_unicode('C') \
}

#define USB_SN_LENGTH         13
#define USB_SERIAL_NUMBER \
{\
  Usb_unicode('1'),\
  Usb_unicode('.'),\
  Usb_unicode('0'),\
  Usb_unicode('.'),\
  Usb_unicode('0'),\
  Usb_unicode('.'),\
  Usb_unicode('0'),\
  Usb_unicode('.'),\
  Usb_unicode('0'),\
  Usb_unicode('.'),\
  Usb_unicode('0'),\
  Usb_unicode('.'),\
  Usb_unicode('A') \
}

#define LANGUAGE_ID           0x0409

typedef struct
{
   U32 dwDTERate;
   U8 bCharFormat;
   U8 bParityType;
   U8 bDataBits;
}S_line_coding;

//! USB Request
typedef struct __attribute__((__packed__))
{
  U8      bmRequestType;        //!< Characteristics of the request
  U8      bRequest;             //!< Specific request
  U16     wValue;               //!< Field that varies according to request
  U16     wIndex;               //!< Field that varies according to request
  U16     wLength;              //!< Number of bytes to transfer if Data
} S_UsbRequest;

//! USB Device Descriptor
typedef struct __attribute__((__packed__))
{
  U8      bLength;              //!< Size of this descriptor in bytes
  U8      bDescriptorType;      //!< DEVICE descriptor type
  U16     bscUSB;               //!< Binay Coded Decimal Spec. release
  U8      bDeviceClass;         //!< Class code assigned by the USB
  U8      bDeviceSubClass;      //!< Subclass code assigned by the USB
  U8      bDeviceProtocol;      //!< Protocol code assigned by the USB
  U8      bMaxPacketSize0;      //!< Max packet size for EP0
  U16     idVendor;             //!< Vendor ID. ATMEL = 0x03EB
  U16     idProduct;            //!< Product ID assigned by the manufacturer
  U16     bcdDevice;            //!< Device release number
  U8      iManufacturer;        //!< Index of manu. string descriptor
  U8      iProduct;             //!< Index of prod. string descriptor
  U8      iSerialNumber;        //!< Index of S.N.  string descriptor
  U8      bNumConfigurations;   //!< Number of possible configurations
} S_usb_device_descriptor;

//! USB Configuration Descriptor
typedef struct __attribute__((__packed__))
{
  U8      bLength;              //!< Size of this descriptor in bytes
  U8      bDescriptorType;      //!< CONFIGURATION descriptor type
  U16     wTotalLength;         //!< Total length of data returned
  U8      bNumInterfaces;       //!< Number of interfaces for this conf.
  U8      bConfigurationValue;  //!< Value for SetConfiguration resquest
  U8      iConfiguration;       //!< Index of string descriptor
  U8      bmAttributes;         //!< Configuration characteristics
  U8      MaxPower;             //!< Maximum power consumption
} S_usb_configuration_descriptor;

//! USB Interface Descriptor
typedef struct __attribute__((__packed__))
{
  U8      bLength;              //!< Size of this descriptor in bytes
  U8      bDescriptorType;      //!< INTERFACE descriptor type
  U8      bInterfaceNumber;     //!< Number of interface
  U8      bAlternateSetting;    //!< Value to select alternate setting
  U8      bNumEndpoints;        //!< Number of EP except EP 0
  U8      bInterfaceClass;      //!< Class code assigned by the USB
  U8      bInterfaceSubClass;   //!< Subclass code assigned by the USB
  U8      bInterfaceProtocol;   //!< Protocol code assigned by the USB
  U8      iInterface;           //!< Index of string descriptor
} S_usb_interface_descriptor;

//! USB Endpoint Descriptor
typedef struct __attribute__((__packed__))
{
  U8      bLength;              //!< Size of this descriptor in bytes
  U8      bDescriptorType;      //!< ENDPOINT descriptor type
  U8      bEndpointAddress;     //!< Address of the endpoint
  U8      bmAttributes;         //!< Endpoint's attributes
  U16     wMaxPacketSize;       //!< Maximum packet size for this EP
  U8      bInterval;            //!< Interval for polling EP in ms
} S_usb_endpoint_descriptor;


//! USB Device Qualifier Descriptor
typedef struct __attribute__((__packed__))
{
  U8      bLength;              //!< Size of this descriptor in bytes
  U8      bDescriptorType;      //!< Device Qualifier descriptor type
  U16     bscUSB;               //!< Binay Coded Decimal Spec. release
  U8      bDeviceClass;         //!< Class code assigned by the USB
  U8      bDeviceSubClass;      //!< Subclass code assigned by the USB
  U8      bDeviceProtocol;      //!< Protocol code assigned by the USB
  U8      bMaxPacketSize0;      //!< Max packet size for EP0
  U8      bNumConfigurations;   //!< Number of possible configurations
  U8      bReserved;            //!< Reserved for future use, must be zero
} S_usb_device_qualifier_descriptor;



//! USB Language Descriptor
typedef struct __attribute__((__packed__))
{
  U8      bLength;              //!< Size of this descriptor in bytes
  U8      bDescriptorType;      //!< STRING descriptor type
  U16     wlangid;              //!< Language id
} S_usb_language_id;


//_____ U S B   M A N U F A C T U R E R   D E S C R I P T O R _______________

//! struct usb_st_manufacturer
typedef struct __attribute__((__packed__))
{
  U8  bLength;                  //!< Size of this descriptor in bytes
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_MN_LENGTH];   //!< Unicode characters
} S_usb_manufacturer_string_descriptor;


//_____ U S B   P R O D U C T   D E S C R I P T O R _________________________

//! struct usb_st_product
typedef struct __attribute__((__packed__))
{
  U8  bLength;                  //!< Size of this descriptor in bytes
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_PN_LENGTH];   //!< Unicode characters
} S_usb_product_string_descriptor;


//_____ U S B   S E R I A L   N U M B E R   D E S C R I P T O R _____________

//! struct usb_st_serial_number
typedef struct __attribute__((__packed__))
{
  U8  bLength;                  //!< Size of this descriptor in bytes
  U8  bDescriptorType;          //!< STRING descriptor type
  U16 wstring[USB_SN_LENGTH];   //!< Unicode characters
} S_usb_serial_number;


//_____ U S B   D E V I C E   C D C   D E S C R I P T O R ___________________

typedef struct __attribute__((__packed__))
{
  S_usb_configuration_descriptor cfg;
  S_usb_interface_descriptor     ifc0;
  U8 CS_INTERFACE1[19];
  S_usb_endpoint_descriptor      ep3;
  S_usb_interface_descriptor     ifc1;
  S_usb_endpoint_descriptor      ep1;
  S_usb_endpoint_descriptor      ep2;
} S_usb_user_configuration_descriptor;

/* ----------------------------------------------------------------------------- */
// usb_user_device_descriptor
const S_usb_device_descriptor usb_dev_desc =
{
  sizeof(S_usb_device_descriptor),
  DEVICE_DESCRIPTOR,
  Usb_format_mcu_to_usb_data(16, USB_SPECIFICATION),
  DEVICE_CLASS,
  DEVICE_SUB_CLASS,
  DEVICE_PROTOCOL,
  EP_CONTROL_LENGTH,
  Usb_format_mcu_to_usb_data(16, VENDOR_ID),
  Usb_format_mcu_to_usb_data(16, PRODUCT_ID),
  Usb_format_mcu_to_usb_data(16, RELEASE_NUMBER),
  MAN_INDEX,
  PROD_INDEX,
  SN_INDEX,
  NB_CONFIGURATION
};


// usb_user_configuration_descriptor FS
const S_usb_user_configuration_descriptor usb_conf_desc_fs =
{
   {  sizeof(S_usb_configuration_descriptor)
   ,  CONFIGURATION_DESCRIPTOR
   ,  Usb_format_mcu_to_usb_data(16, sizeof(S_usb_user_configuration_descriptor))
   ,  NB_INTERFACE
   ,  CONF_NB
   ,  CONF_INDEX
   ,  CONF_ATTRIBUTES
   ,  MAX_POWER
   }
,  {  sizeof(S_usb_interface_descriptor)
   ,  INTERFACE_DESCRIPTOR
   ,  INTERFACE_NB_0
   ,  ALTERNATE_0
   ,  NB_ENDPOINT_0
   ,  INTERFACE_CLASS_0
   ,  INTERFACE_SUB_CLASS_0
   ,  INTERFACE_PROTOCOL_0
   ,  INTERFACE_INDEX_0
   }
,  { 0x05,       // Size of structure
                 // -----------------
     0x24,       // CS_INTERFACE
     0x00,       // Header Functional Descriptor
     0x10, 0x01, // USB Class Definitions for Communication Devices Specification release number in
                 // binary-coded decimal.

     0x05,       // Size of structure
                 // -----------------
     0x24,       // CS_INTERFACE
     0x01,       // Call Management Functional Descriptor
     0x03,       // The capabilities that this configuration supports:
                 // - Device handles call management itself.
                 // - Device can send/receive call management information over a Data Class interface.
     0x01,       // Interface number of Data Class interface optionally used for call management.

     0x04,       // Size of structure
                 // -----------------
     0x24,       // CS_INTERFACE
     0x02,       // Abstract Control Management Functional Descriptor.
     0x06,       // Abstract Control Management functional descriptor subtype:
                 // - Union Functional descriptor

     0x05,       // Size of structure
                 // -----------------
     0x24,       // CS_INTERFACE
     0x06,       // Union Functional descriptor
     0x00,       // The interface number of the Communication or Data Class interface, designated as
                 // the master or controlling interface for the union.
     0x01        // Interface number of first slave or associated interface in the union.
   }
,  {  sizeof(S_usb_endpoint_descriptor)
   ,  ENDPOINT_DESCRIPTOR
   ,  ENDPOINT_NB_3
   ,  EP_ATTRIBUTES_3
   ,  Usb_format_mcu_to_usb_data(16, EP_SIZE_3)
   ,  EP_INTERVAL_3
   }
,  {  sizeof(S_usb_interface_descriptor)
   ,  INTERFACE_DESCRIPTOR
   ,  INTERFACE_NB_1
   ,  ALTERNATE_1
   ,  NB_ENDPOINT_1
   ,  INTERFACE_CLASS_1
   ,  INTERFACE_SUB_CLASS_1
   ,  INTERFACE_PROTOCOL_1
   ,  INTERFACE_INDEX_1
   }
,  {  sizeof(S_usb_endpoint_descriptor)
   ,  ENDPOINT_DESCRIPTOR
   ,  ENDPOINT_NB_1
   ,  EP_ATTRIBUTES_1
   ,  Usb_format_mcu_to_usb_data(16, EP_SIZE_1_FS)
   ,  EP_INTERVAL_1
   }
,  {  sizeof(S_usb_endpoint_descriptor)
   ,  ENDPOINT_DESCRIPTOR
   ,  ENDPOINT_NB_2
   ,  EP_ATTRIBUTES_2
   ,  Usb_format_mcu_to_usb_data(16, EP_SIZE_2_FS)
   ,  EP_INTERVAL_2
   }
};


// usb_user_language_id
const S_usb_language_id usb_user_language_id =
{
  sizeof(S_usb_language_id),
  STRING_DESCRIPTOR,
  Usb_format_mcu_to_usb_data(16, LANGUAGE_ID)
};


// usb_user_manufacturer_string_descriptor
const S_usb_manufacturer_string_descriptor usb_user_manufacturer_string_descriptor =
{
  sizeof(S_usb_manufacturer_string_descriptor),
  STRING_DESCRIPTOR,
  USB_MANUFACTURER_NAME
};


// usb_user_product_string_descriptor
const S_usb_product_string_descriptor usb_user_product_string_descriptor =
{
  sizeof(S_usb_product_string_descriptor),
  STRING_DESCRIPTOR,
  USB_PRODUCT_NAME
};


// usb_user_serial_number
const S_usb_serial_number usb_user_serial_number =
{
  sizeof(S_usb_serial_number),
  STRING_DESCRIPTOR,
  USB_SERIAL_NUMBER
};

extern Bool usb_user_read_request(U8, U8);
extern Bool usb_user_get_descriptor(U8, U8);
void        cdc_get_line_coding(void);
void        cdc_set_line_coding(void);
void        cdc_set_control_line_state (void);

__attribute__((__interrupt__)) void usb_general_interrupt(void);
//void    usb_device_task_init(void);
void    Usb_sof_action(void);
void    usb_start_device(void);
void    usb_process_request(void);
//void    device_cdc_task_init(void);
void    usb_init(void);
void    usb_device_task(void);
Bool    usb_user_get_descriptor(U8 type, U8 string);
Bool    usb_user_read_request(U8 type, U8 request);
void    usb_user_endpoint_init(U8 conf_nb);

extern  Status_bool_t   usb_init_device         (                             void   );
extern  U32             usb_set_ep_txpacket     (U8,         U8  , U32               );
extern  U32             usb_write_ep_txpacket   (U8, const void *, U32, const void **);
extern  U32             usb_read_ep_rxpacket    (U8,       void *, U32,       void **);

extern volatile       U8   usb_configuration_nb;

void    UsbCdcFlush (void);
int     UsbCdcSendChar(int);
int     UsbCdcReadChar( int *);
Bool    UsbCdcTxReady(void);
Bool    UsbCdcTestHit(void);

#endif
