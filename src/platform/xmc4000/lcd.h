#ifndef LCD_H_
#define LCD_H_

/*
 * Arduino's LiquidCrystal interface: Adapted for XMC4300 (and
 * friends) by Raman Gopalan <ramangopalan@gmail.com>
 * See https://github.com/arduino-libraries/LiquidCrystal
 *
 * LCD Lua module structure adapted from Mizar32's I2C LCD interface.
 */

#include <inttypes.h>
#include <DAVE.h>

/* Number of rows and columns */
#define LCD_COLS                   (16U)
#define LCD_ROWS                   (2U)

/* Commands */
#define LCD_CLEARDISPLAY           (0x01)
#define LCD_RETURNHOME             (0x02)
#define LCD_ENTRYMODESET           (0x04)
#define LCD_DISPLAYCONTROL         (0x08)
#define LCD_CURSORSHIFT            (0x10)
#define LCD_FUNCTIONSET            (0x20)
#define LCD_SETCGRAMADDR           (0x40)
#define LCD_SETDDRAMADDR           (0x80)

/* Flags for display entry mode */
#define LCD_ENTRYRIGHT             (0x00)
#define LCD_ENTRYLEFT              (0x02)
#define LCD_ENTRYSHIFTINCREMENT    (0x01)
#define LCD_ENTRYSHIFTDECREMENT    (0x00)

/* Flags for display on/off control */
#define LCD_DISPLAYON              (0x04)
#define LCD_DISPLAYOFF             (0x00)
#define LCD_CURSORON               (0x02)
#define LCD_CURSOROFF              (0x00)
#define LCD_BLINKON                (0x01)
#define LCD_BLINKOFF               (0x00)

/* Flags for display/cursor shift */
#define LCD_DISPLAYMOVE            (0x08)
#define LCD_CURSORMOVE             (0x00)
#define LCD_MOVERIGHT              (0x04)
#define LCD_MOVELEFT               (0x00)

/* Flags for function set */
#define LCD_8BITMODE               (0x10)
#define LCD_4BITMODE               (0x00)
#define LCD_2LINE                  (0x08)
#define LCD_1LINE                  (0x00)
#define LCD_5x10DOTS               (0x04)
#define LCD_5x8DOTS                (0x00)

/* HW pins */
#define LCD_RS_PIN                 P3_6
#define LCD_ENABLE_PIN             P0_6
#define LCD_DATA4_PIN              P5_0
#define LCD_DATA5_PIN              P2_7
#define LCD_DATA6_PIN              P2_9
#define LCD_DATA7_PIN              P2_4 //P15_9

/********************************************************************************
   STRUCTURES
 *******************************************************************************/

typedef struct lcd_t {
  uint8_t rs_pin; XMC_GPIO_PORT_t *rs_port;
  uint8_t rw_pin; XMC_GPIO_PORT_t *rw_port;
  uint8_t enable_pin; XMC_GPIO_PORT_t *enable_port;
  uint8_t data_pins[8]; XMC_GPIO_PORT_t *data_ports[8];
  uint8_t display_function;
  uint8_t display_control;
  uint8_t display_mode;
  uint8_t initialized;
  uint8_t num_lines;
  uint8_t row_offsets[4];
} lcd_t;

/********************************************************************************
   FUNCTION PROTOTYPES
 *******************************************************************************/

/* LCD system commands */
void lcd_set_row_offsets (lcd_t *l, int row0, int row1, int row2, int row3);
void lcd_set_cursor (lcd_t *l, uint8_t col, uint8_t row);
size_t lcd_write (lcd_t *l, uint8_t value);
void lcd_command (lcd_t *l, uint8_t value);
void lcd_send (lcd_t *l, uint8_t value, uint8_t mode);
void lcd_write4bits (lcd_t *l, uint8_t value);
void lcd_write8bits (lcd_t *l, uint8_t value);
void lcd_pulse_enable (lcd_t *l);
void lcd_begin (lcd_t *l, uint8_t cols, uint8_t lines, uint8_t dotsize);

/* LCD user commands */
void lcd_create_char (lcd_t *l, uint8_t location, uint8_t charmap[]);
void lcd_init_4bits_mode (lcd_t *l);
void lcd_echo (lcd_t *l, uint8_t *s, uint32_t len);
void lcd_clear (lcd_t *l);
void lcd_home (lcd_t *l);
void lcd_no_display (lcd_t *l);
void lcd_display (lcd_t *l);
void lcd_no_blink (lcd_t *l);
void lcd_blink (lcd_t *l);
void lcd_no_cursor (lcd_t *l);
void lcd_cursor (lcd_t *l);
void lcd_scroll_display_left (lcd_t *l);
void lcd_scroll_display_right (lcd_t *l);
void lcd_left_to_right (lcd_t *l);
void lcd_right_to_left (lcd_t *l);
void lcd_autoscroll (lcd_t *l);
void lcd_no_autoscroll (lcd_t *l);
void lcd_init (lcd_t *l,
               uint8_t four_bit_mode,
               XMC_GPIO_PORT_t *const rs_port,
               uint8_t rs_pin,
               XMC_GPIO_PORT_t *const rw_port,
               uint8_t rw_pin,
               XMC_GPIO_PORT_t *const enable_port,
               uint8_t enable_pin,
               XMC_GPIO_PORT_t *const d0_port,
               uint8_t d0_pin,
               XMC_GPIO_PORT_t *const d1_port,
               uint8_t d1_pin,
               XMC_GPIO_PORT_t *const d2_port,
               uint8_t d2_pin,
               XMC_GPIO_PORT_t *const d3_port,
               uint8_t d3_pin,
               XMC_GPIO_PORT_t *const d4_port,
               uint8_t d4_pin,
               XMC_GPIO_PORT_t *const d5_port,
               uint8_t d5_pin,
               XMC_GPIO_PORT_t *const d6_port,
               uint8_t d6_pin,
               XMC_GPIO_PORT_t *const d7_port,
               uint8_t d7_pin);

#endif /* #ifndef LCD_H_ */
