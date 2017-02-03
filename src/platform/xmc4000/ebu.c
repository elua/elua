
/*
 * Copyright (C) 2015 Infineon Technologies AG. All rights reserved.
 *
 * Infineon Technologies AG (Infineon) is supplying this software for use with
 * Infineon's microcontrollers.
 * This file can be freely distributed within development tools that are
 * supporting such microcontrollers.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS". NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * INFINEON SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 * OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 */

/*
 * Modified by Mounika Munipalli, Sneha Lakshmipathi and Raman Gopalan
 * for getting Dmitry Grinberg's uARM running on the XMC4500.
 */

/**
 * @file
 * @date 20 February,2016
 * @version 1.0.2
 *
 * @brief EBU demo example
 *
 * This example demonstrates proper initialization of external SDRAM and R/W Operation
 *
 * History <br>
 * - CAS latency corrected to 3
 * - example code is now writing/reading 32bit values instead of 16 bit
 *
 * Version 1.0.0 Initial <br>
 *
 *
 */

#if defined ( XMC4500_E144x1024 )

#include <xmc_gpio.h>
#include <xmc_ebu.h>

#define LED1 P5_2

#define SDRAM_CKE  P5_3
#define SDRAM_CLK  P6_4
#define SDRAM_bCS  P3_2
#define SDRAM_bRAS P5_4
#define SDRAM_bCAS P5_5
#define SDRAM_bWE  P3_1

#define SDRAM_A0   P1_12
#define SDRAM_A1   P1_13
#define SDRAM_A2   P1_14
#define SDRAM_A3   P1_15
#define SDRAM_A4   P2_0
#define SDRAM_A5   P2_1
#define SDRAM_A6   P2_2
#define SDRAM_A7   P2_3
#define SDRAM_A8   P2_4
#define SDRAM_A9   P2_5
#define SDRAM_A10  P2_8
#define SDRAM_A11  P2_9
#define SDRAM_BA0  P2_10
#define SDRAM_BA1  P2_11

#define SDRAM_DQ0  P0_2
#define SDRAM_DQ1  P0_3
#define SDRAM_DQ2  P0_4
#define SDRAM_DQ3  P0_5
#define SDRAM_DQ4  P3_5
#define SDRAM_DQ5  P3_6
#define SDRAM_DQ6  P0_7
#define SDRAM_DQ7  P0_8
#define SDRAM_DQ8  P4_0
#define SDRAM_DQ9  P4_1
#define SDRAM_DQ10 P1_6
#define SDRAM_DQ11 P1_7
#define SDRAM_DQ12 P1_8
#define SDRAM_DQ13 P1_9
#define SDRAM_DQ14 P1_2
#define SDRAM_DQ15 P1_3

#define SDRAM_UDQM P2_15
#define SDRAM_LDQM P2_14

/******************************* EBU Configuraiton ***************************************************/

XMC_EBU_t *const ebumodule = (XMC_EBU_t *)EBU;

XMC_EBU_CONFIG_t ebuobj = {
  .ebu_clk_config.ebu_clock_divide_ratio = XMC_EBU_CLOCK_DIVIDED_BY_2,
  .ebu_clk_config.ebu_div2_clk_mode = XMC_EBU_DIV2_CLK_MODE_ON,
  .ebu_clk_config.ebu_clk_mode = XMC_EBU_CLK_MODE_SYNCHRONOUS_TO_CPU,
  .ebu_mode_config.ebu_sdram_tristate = false,
  .ebu_mode_config.ebu_extlock = false,
  .ebu_mode_config.ebu_arbsync = true,
  .ebu_mode_config.ebu_arbitration_mode = XMC_EBU_ARB_MODE_SOLE_MASTER_MODE,
  .ebu_mode_config.bus_timeout_control = 0xFFU,
  .ebu_mode_config.ebu_ale_mode = XMC_EBU_ALE_OUTPUT_IS_INV_ADV,
  .ebu_free_pins_to_gpio.address_pins_gpio = 0x1ff,
  .ebu_free_pins_to_gpio.adv_pin_gpio = false
};

XMC_EBU_REGION_t ebureadwriteconfig = {
  .read_config.ebu_region_no = 0x0U,
  .read_config.ebu_bus_read_config.ebu_burst_length_sync = 0x4U,
  .read_config.ebu_bus_read_config.ebu_byte_control =  XMC_EBU_BYTE_CONTROL_FOLLOWS_CONTROL_SIGNAL_TIMMING,
  .read_config.ebu_bus_read_config.ebu_burst_flash_clock_feedback = XMC_EBU_BURST_FLASH_CLOCK_FEEDBACK_ENABLE,
  .read_config.ebu_bus_read_config.ebu_device_addressing_mode = XMC_EBU_DEVICE_ADDRESSING_MODE_16_BITS,
  .read_config.ebu_bus_read_config.ebu_device_type = XMC_EBU_DEVICE_TYPE_SDRAM,
  .read_config.ebu_bus_read_config.address_cycles = 0xFU,
  .read_config.ebu_bus_read_config.address_hold_cycles = 0xFU,
  .read_config.ebu_bus_read_config.address_hold_cycles = 0xFU,
  .read_config.ebu_bus_read_config.command_delay_lines = 0xFU,
  .read_config.ebu_bus_read_config.ebu_ext_data = 0x0U,
  .read_config.ebu_bus_read_config.ebu_freq_ext_clk_pin = 0x0U,
  .read_config.ebu_bus_read_config.ebu_recovery_cycles_between_different_regions  = 0xFU,
  .read_config.ebu_bus_read_config.ebu_recovery_cycles_after_read_accesses  = 0x7U,
  .read_config.ebu_bus_read_config.ebu_programmed_wait_states_for_read_accesses = 0x4U,
  .write_config.ebu_region_no = 0x0U,
  .write_config.ebu_bus_write_config.ebu_burst_length_sync = 0x04U,
  .write_config.ebu_bus_write_config.ebu_byte_control =  XMC_EBU_BYTE_CONTROL_FOLLOWS_CONTROL_SIGNAL_TIMMING,
  .write_config.ebu_bus_write_config.ebu_device_type = XMC_EBU_DEVICE_TYPE_SDRAM,
  .write_config.ebu_bus_write_config.address_cycles = 0xFU,
  .write_config.ebu_bus_write_config.address_hold_cycles = 0xFU,
  .write_config.ebu_bus_write_config.command_delay_lines = 0xFU,
  .write_config.ebu_bus_write_config.ebu_ext_data = 0x0U,
  .write_config.ebu_bus_write_config.ebu_freq_ext_clk_pin = 0x0U,
  .write_config.ebu_bus_write_config.ebu_recovery_cycles_between_different_regions  = 0xFU,
  .write_config.ebu_bus_write_config.ebu_recovery_cycles_after_write_accesses  = 0x7U,
  .write_config.ebu_bus_write_config.ebu_programmed_wait_states_for_write_accesses = 0x4U,
};

XMC_EBU_REGION_t ebureadwriteconfig_1 = {
  .read_config.ebu_region_no = 0x1U,
  .read_config.ebu_bus_read_config.ebu_burst_length_sync = 0x4U,
  .read_config.ebu_bus_read_config.ebu_byte_control =  XMC_EBU_BYTE_CONTROL_FOLLOWS_CONTROL_SIGNAL_TIMMING,
  .read_config.ebu_bus_read_config.ebu_burst_flash_clock_feedback = XMC_EBU_BURST_FLASH_CLOCK_FEEDBACK_ENABLE,
  .read_config.ebu_bus_read_config.ebu_device_addressing_mode = XMC_EBU_DEVICE_ADDRESSING_MODE_16_BITS,
  .read_config.ebu_bus_read_config.ebu_device_type = XMC_EBU_DEVICE_TYPE_SDRAM,
  .read_config.ebu_bus_read_config.address_cycles = 0xFU,
  .read_config.ebu_bus_read_config.address_hold_cycles = 0xFU,
  .read_config.ebu_bus_read_config.address_hold_cycles = 0xFU,
  .read_config.ebu_bus_read_config.command_delay_lines = 0xFU,
  .read_config.ebu_bus_read_config.ebu_ext_data = 0x0U,
  .read_config.ebu_bus_read_config.ebu_freq_ext_clk_pin = 0x0U,
  .read_config.ebu_bus_read_config.ebu_recovery_cycles_between_different_regions  = 0xFU,
  .read_config.ebu_bus_read_config.ebu_recovery_cycles_after_read_accesses  = 0x7U,
  .read_config.ebu_bus_read_config.ebu_programmed_wait_states_for_read_accesses = 0x4U,
  .write_config.ebu_region_no = 0x1U,
  .write_config.ebu_bus_write_config.ebu_burst_length_sync = 0x04U,
  .write_config.ebu_bus_write_config.ebu_byte_control =  XMC_EBU_BYTE_CONTROL_FOLLOWS_CONTROL_SIGNAL_TIMMING,
  .write_config.ebu_bus_write_config.ebu_device_type = XMC_EBU_DEVICE_TYPE_SDRAM,
  .write_config.ebu_bus_write_config.address_cycles = 0xFU,
  .write_config.ebu_bus_write_config.address_hold_cycles = 0xFU,
  .write_config.ebu_bus_write_config.command_delay_lines = 0xFU,
  .write_config.ebu_bus_write_config.ebu_ext_data = 0x0U,
  .write_config.ebu_bus_write_config.ebu_freq_ext_clk_pin = 0x0U,
  .write_config.ebu_bus_write_config.ebu_recovery_cycles_between_different_regions  = 0xFU,
  .write_config.ebu_bus_write_config.ebu_recovery_cycles_after_write_accesses  = 0x7U,
  .write_config.ebu_bus_write_config.ebu_programmed_wait_states_for_write_accesses = 0x4U,
};

XMC_EBU_REGION_t ebureadwriteconfig_2 = {
  .read_config.ebu_region_no = 0x2U,
  .read_config.ebu_bus_read_config.ebu_burst_length_sync = 0x4U,
  .read_config.ebu_bus_read_config.ebu_byte_control =  XMC_EBU_BYTE_CONTROL_FOLLOWS_CONTROL_SIGNAL_TIMMING,
  .read_config.ebu_bus_read_config.ebu_burst_flash_clock_feedback = XMC_EBU_BURST_FLASH_CLOCK_FEEDBACK_ENABLE,
  .read_config.ebu_bus_read_config.ebu_device_addressing_mode = XMC_EBU_DEVICE_ADDRESSING_MODE_16_BITS,
  .read_config.ebu_bus_read_config.ebu_device_type = XMC_EBU_DEVICE_TYPE_SDRAM,
  .read_config.ebu_bus_read_config.address_cycles = 0xFU,
  .read_config.ebu_bus_read_config.address_hold_cycles = 0xFU,
  .read_config.ebu_bus_read_config.address_hold_cycles = 0xFU,
  .read_config.ebu_bus_read_config.command_delay_lines = 0xFU,
  .read_config.ebu_bus_read_config.ebu_ext_data = 0x0U,
  .read_config.ebu_bus_read_config.ebu_freq_ext_clk_pin = 0x0U,
  .read_config.ebu_bus_read_config.ebu_recovery_cycles_between_different_regions  = 0xFU,
  .read_config.ebu_bus_read_config.ebu_recovery_cycles_after_read_accesses  = 0x7U,
  .read_config.ebu_bus_read_config.ebu_programmed_wait_states_for_read_accesses = 0x4U,
  .write_config.ebu_region_no = 0x2U,
  .write_config.ebu_bus_write_config.ebu_burst_length_sync = 0x04U,
  .write_config.ebu_bus_write_config.ebu_byte_control =  XMC_EBU_BYTE_CONTROL_FOLLOWS_CONTROL_SIGNAL_TIMMING,
  .write_config.ebu_bus_write_config.ebu_device_type = XMC_EBU_DEVICE_TYPE_SDRAM,
  .write_config.ebu_bus_write_config.address_cycles = 0xFU,
  .write_config.ebu_bus_write_config.address_hold_cycles = 0xFU,
  .write_config.ebu_bus_write_config.command_delay_lines = 0xFU,
  .write_config.ebu_bus_write_config.ebu_ext_data = 0x0U,
  .write_config.ebu_bus_write_config.ebu_freq_ext_clk_pin = 0x0U,
  .write_config.ebu_bus_write_config.ebu_recovery_cycles_between_different_regions  = 0xFU,
  .write_config.ebu_bus_write_config.ebu_recovery_cycles_after_write_accesses  = 0x7U,
  .write_config.ebu_bus_write_config.ebu_programmed_wait_states_for_write_accesses = 0x4U,
};

XMC_EBU_REGION_t ebureadwriteconfig_3 = {
  .read_config.ebu_region_no = 0x3U,
  .read_config.ebu_bus_read_config.ebu_burst_length_sync = 0x4U,
  .read_config.ebu_bus_read_config.ebu_byte_control =  XMC_EBU_BYTE_CONTROL_FOLLOWS_CONTROL_SIGNAL_TIMMING,
  .read_config.ebu_bus_read_config.ebu_burst_flash_clock_feedback = XMC_EBU_BURST_FLASH_CLOCK_FEEDBACK_ENABLE,
  .read_config.ebu_bus_read_config.ebu_device_addressing_mode = XMC_EBU_DEVICE_ADDRESSING_MODE_16_BITS,
  .read_config.ebu_bus_read_config.ebu_device_type = XMC_EBU_DEVICE_TYPE_SDRAM,
  .read_config.ebu_bus_read_config.address_cycles = 0xFU,
  .read_config.ebu_bus_read_config.address_hold_cycles = 0xFU,
  .read_config.ebu_bus_read_config.address_hold_cycles = 0xFU,
  .read_config.ebu_bus_read_config.command_delay_lines = 0xFU,
  .read_config.ebu_bus_read_config.ebu_ext_data = 0x0U,
  .read_config.ebu_bus_read_config.ebu_freq_ext_clk_pin = 0x0U,
  .read_config.ebu_bus_read_config.ebu_recovery_cycles_between_different_regions  = 0xFU,
  .read_config.ebu_bus_read_config.ebu_recovery_cycles_after_read_accesses  = 0x7U,
  .read_config.ebu_bus_read_config.ebu_programmed_wait_states_for_read_accesses = 0x4U,
  .write_config.ebu_region_no = 0x3U,
  .write_config.ebu_bus_write_config.ebu_burst_length_sync = 0x04U,
  .write_config.ebu_bus_write_config.ebu_byte_control =  XMC_EBU_BYTE_CONTROL_FOLLOWS_CONTROL_SIGNAL_TIMMING,
  .write_config.ebu_bus_write_config.ebu_device_type = XMC_EBU_DEVICE_TYPE_SDRAM,
  .write_config.ebu_bus_write_config.address_cycles = 0xFU,
  .write_config.ebu_bus_write_config.address_hold_cycles = 0xFU,
  .write_config.ebu_bus_write_config.command_delay_lines = 0xFU,
  .write_config.ebu_bus_write_config.ebu_ext_data = 0x0U,
  .write_config.ebu_bus_write_config.ebu_freq_ext_clk_pin = 0x0U,
  .write_config.ebu_bus_write_config.ebu_recovery_cycles_between_different_regions  = 0xFU,
  .write_config.ebu_bus_write_config.ebu_recovery_cycles_after_write_accesses  = 0x7U,
  .write_config.ebu_bus_write_config.ebu_programmed_wait_states_for_write_accesses = 0x4U,
};

XMC_EBU_SDRAM_CONFIG_t ebusdramcontrol =  {
  .ebu_sdram_clk_mode= XMC_EBU_SDRAM_CLK_MODE_CONTINUOUSLY_RUNS,                    /**< SDRAM clock mode select */
  .ebu_sdram_mask_for_bank_tag = XMC_EBU_SDRAM_MASK_FOR_BANK_TAG_ADDRESS_22_to_21,//XMC_EBU_SDRAM_MASK_FOR_BANK_TAG_ADDRESS_22_to_21,  /**< Mask for Bank Tag */
  .ebu_sdram_mask_for_row_tag = XMC_EBU_SDRAM_WIDTH_OF_COLUMN_ADDRESS_8_to_0,       /**< Mask for Row Tag */
  /**< Row cycle time counter: Insert (CRCE * 8) + CRC + 1 NOP cycles */
  .ebu_sdram_row_cycle_time_counter = 0x5UL,
  /**< (CRCD) Number of NOP cycles between a row address and a column address: Insert CRCD + 1 NOP cycles */
  .ebu_sdram_row_to_column_delay_counter = 0x01U,
  /**< Number of address bits from bit 0 to be used for column address */
  .ebu_sdram_width_of_column_address = XMC_EBU_SDRAM_WIDTH_OF_COLUMN_ADDRESS_8_to_0,
  /**< (CRP) Number of NOP cycles inserted after a precharge command: Insert CRP + 1 NOP cycles */
  .ebu_row_precharge_time_counter= 0x1U,
  /**< (CRSC) Number of NOP cycles after a mode register set command: Insert CRSC + 1 NOP cycles */
  .ebu_mode_register_set_up_time = 0x0U,
  /**< (CRFSH) Number of refresh commands issued during powerup init sequence: Perform CRFSH + 1 refresh cycles */
  .ebu_init_refresh_commands_counter = 0x0AU,
  /**< Number of clock cycles between row activate command and a precharge command */
  .ebu_row_precharge_delay_counter = 0x0AU, //0x03U,
  .ebu_sdram_burst_length = XMC_EBU_SDRAM_BURST_LENGTH_4_LOCATION,
  .ebu_sdram_casclk_mode = 0x3U,
  .ebu_sdram_cold_start = 0x1U,
  .ebu_sdram_self_refresh_exit = true,
  .ebu_sdram_num_refresh_counter_period = 0x2U,
  .ebu_sdram_num_refresh_cmnds = 0x1U,
  .ebu_sdram_auto_refresh = 0x1U,
  .ebu_sdram_self_refresh_exit_delay = 0xFFU,
  .ebu_sdram_auto_self_refresh = 0x01U,
  .ebu_sdram_delay_on_power_down_exit = 0x07U
};

#define TEST_SIZE 2097152  //67108864 bits total size (2^26)/32 = 0x0020 0000

/* Base Address of external RAM memory region */
#define EBU_EXT_RAM_REGION_BASE	  0x60000000UL

/* SDRAM base address assignment */
#define SDRAM_BASE EBU_EXT_RAM_REGION_BASE

/* Delay function */
static void sdram_delay (int n) {
  int i = 0;

  for (i = 0; i < n; i++) {
    __NOP();
  }
}

static int32_t sdram_test_helper (void) {
  uint32_t status = 0;
  uint32_t mem_add = 0x0;
  uint32_t i = 0x0;
  uint32_t value = 0x0;

  /* Writing to SDRAM */
  *(volatile uint32_t *)(SDRAM_BASE) = 0x0;
  *(volatile uint32_t *)(SDRAM_BASE + 0x4) = 0x12345678;
  *(volatile uint32_t *)(SDRAM_BASE + 0x8) = 0x87654321;
  *(volatile uint32_t *)(SDRAM_BASE + 0xC) = 0xAAAA5555;
  *(volatile uint32_t *)(SDRAM_BASE + 0x10) = 0xFF0000FF;

  /* Reading from SD RAM*/
  value = *(volatile uint32_t *)(SDRAM_BASE + 0x0);
  if (value != 0x0) {
    status = 1;
  }
  value = *(volatile uint32_t *)(SDRAM_BASE + 0x4);
  if (value != 0x12345678) {
    status = 1;
  }
  value = *(volatile uint32_t *)(SDRAM_BASE + 0x8);
  if (value != 0x87654321) {
    status = 1;
  }
  value = *(volatile uint32_t *)(SDRAM_BASE + 0xC);
  if (value != 0xAAAA5555) {
    status = 1;
  }
  value = *(volatile uint32_t *)(SDRAM_BASE + 0x10);
  if (value != 0xFF0000FF) {
    status = 1;
  }

  for (i = 0; i < TEST_SIZE; i++) {            //16777216 total size (2^26)/4
    mem_add = SDRAM_BASE + (i*4);
    *(volatile uint32_t *)mem_add = (i);
  }

  for (i = 0; i < TEST_SIZE ; i++) {
    mem_add = SDRAM_BASE + (4 * i);
    value = *(volatile uint32_t *)(mem_add);
    if ((value != (i))) {
      status = 1;
    }
  }

  return status;
}

/* Test SDRAM */
void sdram_test (void) {
  uint32_t count = 0x64U;

  if (sdram_test_helper()) {
    while (count) {
      XMC_GPIO_ToggleOutput(LED1); /* test fail */
      sdram_delay(1000000);
      --count;
    }
  } else {
    XMC_GPIO_SetOutputHigh(LED1); /* test ok (Turn off Pin 5.2) */
  }
}

/*
 * Function to Initialize EBU Registers for Configuration
 */
void EBU_Init (void) {
  XMC_EBU_Init(ebumodule, &ebuobj);

  XMC_EBU_ConfigureRegion(ebumodule, &ebureadwriteconfig);
  XMC_EBU_ConfigureRegion(ebumodule, &ebureadwriteconfig_1);
  XMC_EBU_ConfigureRegion(ebumodule, &ebureadwriteconfig_2);
  XMC_EBU_ConfigureRegion(ebumodule, &ebureadwriteconfig_3);

  XMC_EBU_AddressSelectEnable(ebumodule,XMC_EBU_ADDRESS_SELECT_MEMORY_REGION_ENABLE,0U);
  XMC_EBU_AddressSelectEnable(ebumodule,XMC_EBU_ADDRESS_SELECT_ALTERNATE_REGION_ENABLE,0U);
  XMC_EBU_ConfigureSdram(ebumodule, &ebusdramcontrol);

  XMC_EBU_AddressSelectEnable(ebumodule,XMC_EBU_ADDRESS_SELECT_MEMORY_REGION_ENABLE,1U);
  XMC_EBU_AddressSelectEnable(ebumodule,XMC_EBU_ADDRESS_SELECT_ALTERNATE_REGION_ENABLE,1U);
  XMC_EBU_ConfigureSdram(ebumodule, &ebusdramcontrol);

  XMC_EBU_AddressSelectEnable(ebumodule,XMC_EBU_ADDRESS_SELECT_MEMORY_REGION_ENABLE,2U);
  XMC_EBU_AddressSelectEnable(ebumodule,XMC_EBU_ADDRESS_SELECT_ALTERNATE_REGION_ENABLE,2U);
  XMC_EBU_ConfigureSdram(ebumodule, &ebusdramcontrol);

  XMC_EBU_AddressSelectEnable(ebumodule,XMC_EBU_ADDRESS_SELECT_MEMORY_REGION_ENABLE,3U);
  XMC_EBU_AddressSelectEnable(ebumodule,XMC_EBU_ADDRESS_SELECT_ALTERNATE_REGION_ENABLE,3U);
  XMC_EBU_ConfigureSdram(ebumodule, &ebusdramcontrol);
}

/* EBU port configuration */
void EBU_MUX_Init (void) {
  XMC_GPIO_CONFIG_t config;
  config.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL;
  config.output_level = XMC_GPIO_OUTPUT_LEVEL_LOW;
  config.output_strength = XMC_GPIO_OUTPUT_STRENGTH_STRONG_SHARP_EDGE;

  XMC_GPIO_Init(SDRAM_DQ0, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ0, XMC_GPIO_HWCTRL_PERIPHERAL2);
  
  XMC_GPIO_Init(SDRAM_DQ1, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ1, XMC_GPIO_HWCTRL_PERIPHERAL2);
  
  XMC_GPIO_Init(SDRAM_DQ2, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ2, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ3, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ3, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ4, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ4, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ5, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ5, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ6, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ6, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ7, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ7, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ8, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ8, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ9, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ9, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ10, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ10, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ11, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ11, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ12, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ12, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ13, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ13, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ14, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ14, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_DQ15, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_DQ15, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_A0, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_A0, XMC_GPIO_HWCTRL_PERIPHERAL2);
	
  XMC_GPIO_Init(SDRAM_A1, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_A1, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_A2, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_A2, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_A3, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_A3, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_A4, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_A4, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_A5, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_A5, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_A6, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_A6, XMC_GPIO_HWCTRL_PERIPHERAL2);
  
  XMC_GPIO_Init(SDRAM_A7, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_A7, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_A8, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_A8, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_A9, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_A9, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_A10, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_A10, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_A11, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_A11, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_BA0, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_BA0, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_BA1, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_BA1, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_UDQM, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_UDQM, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_LDQM, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_LDQM, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_bWE, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_bWE, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_bCS, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_bCS, XMC_GPIO_HWCTRL_PERIPHERAL2);

  XMC_GPIO_Init(SDRAM_CKE, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_CKE, XMC_GPIO_HWCTRL_PERIPHERAL1);

  XMC_GPIO_Init(SDRAM_bRAS, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_bRAS, XMC_GPIO_HWCTRL_PERIPHERAL1);

  XMC_GPIO_Init(SDRAM_bCAS, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_bCAS, XMC_GPIO_HWCTRL_PERIPHERAL1);

  XMC_GPIO_Init(SDRAM_CLK, &config);
  XMC_GPIO_SetHardwareControl(SDRAM_CLK, XMC_GPIO_HWCTRL_PERIPHERAL1);
}

void ebu_main (void) {
  /* EBU Clock is divided by 2 - To program the divider connecting a parent and its child clock node*/
  XMC_SCU_CLOCK_SetEbuClockDivider(2U);

  /* Enable EBU Clock */
  XMC_SCU_CLOCK_EnableClock(XMC_SCU_CLOCK_EBU);

  EBU_MUX_Init();
  EBU_Init();

  XMC_GPIO_SetMode(LED1, XMC_GPIO_MODE_OUTPUT_PUSH_PULL);

  sdram_test();
}

#endif // #if defined ( XMC4500_E144x1024 )
