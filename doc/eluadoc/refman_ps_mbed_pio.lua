-- eLua reference manual - platform data

data_en = 
{

  -- Title
  title = "eLua reference manual - MBED pio module",

  -- Menu name
  menu_name = "pio",

  -- Overview
  overview = [[<p>This module provide a function for accessing pin configuration
 features on the LPC17xx family of CPUs, and provides provides a pin
 mapping which corresponds to the pins and LEDs on the MBED platform.
 The pin configuration function allows the user to reconfigure pin
 function (as decribed in section 8.5 of the
 @http://ics.nxp.com/support/documents/microcontrollers/pdf/user.manual.lpc17xx.pdf@lpc17xx user manual@,
 configure the pin as an open drain, and configure the
 pin as pullup, pulldown or tristate.  Note that this module is a
 supplement of the platform independent @refman_gen_pio.html@pio@
 module, not a replacement. Use this module only for setting up the
 MBED PIO pins, and the @refman_gen_pio.html@pio@ module for all the
 other PIO related operations.  Below is a subset of available pin functions:</p>

<ul>
<li>PWM
<table style="text-align: left; margin-left: 2em;">
<tbody>
<tr>
  <th>MBED Pin</th>
  <th>ID</th>
  <th>Function</th>
</tr>
</tr>
  <td>LED1</th>
  <td>1</th>
  <td>FUNCTION_2</th>
</tr>
<tr>
  <td>LED2</th>
  <td>2</th>
  <td>FUNCTION_2</th>
</tr>
<tr>
  <td>LED3</th>
  <td>3</th>
  <td>FUNCTION_2</th>
</tr>
<tr>
  <td>LED4</th>
  <td>4</th>
  <td>FUNCTION_2</th>
</tr>
<tr>
  <td>p21</th>
  <td>6</th>
  <td>FUNCTION_1</th>
</tr>
<tr>
  <td>p22</th>
  <td>5</th>
  <td>FUNCTION_1</th>
</tr>
<tr>
  <td>p23</th>
  <td>4</th>
  <td>FUNCTION_1</th>
</tr>
<tr>
  <td>p24</th>
  <td>3</th>
  <td>FUNCTION_1</th>
</tr>
<tr>
  <td>p25</th>
  <td>2</th>
  <td>FUNCTION_1</th>
</tr>
<tr>
  <td>p26</th>
  <td>1</th>
  <td>FUNCTION_1</th>
</tr>
</tbody>
</table>
</li>

<li>UART
<table style="text-align: left; margin-left: 2em;">
<tbody>
<tr>
  <th>MBED Pin</th>
  <th>ID</th>
  <th>Function</th>
</tr>
<tr>
  <td>p13</th>
  <td>1 (TX)</th>
  <td>FUNCTION_1</th>
</tr>
<tr>
  <td>p14</th>
  <td>1 (RX)</th>
  <td>FUNCTION_1</th>
</tr>
<tr>
  <td>p28</th>
  <td>2 (TX)</th>
  <td>FUNCTION_1</th>
</tr>
<tr>
  <td>p27</th>
  <td>2 (RX)</th>
  <td>FUNCTION_1</th>
</tr>
</tbody>
</table>
</li>
</ul>

 <p>In addition, this module provides pin mapping as found on the mbed
 board so that $mbed.pio.p5$ through $mbed.pio.p30$ map onto the mbed
 pins as described in the
 @http://mbed.org/handbook/mbed-NXP-LPC1768@mbed handbook@. LED pins
 are also usable, for example LED1 would correspond to
 $mbed.pio.LED1$.</p>]],

  -- Functions
  funcs = 
  {
    { sig = "#mbed.pio.configpin#( pin, function, mode, resistor)",
      desc = "Configure the pin.",
      args = 
      {
        "$pin$ - the pin, as encoded in the @refman_gen_pio.html@pio@ or $mbed.pio$ module.",
        "$function$ - alternate pin function, Must be either $mbed.pio.FUNCTION_0 (default)$, $mbed.pio.FUNCTION_1$, $mbed.pio.FUNCTION_2$, or $mbed.pio.FUNCTION_3$",
        "$mode$ - open drain mode, can be either $mbed.pio.MODE_DEFAULT$ (no open drain) or $mbed.pio.MODE_OD$ (open drain).",
        "$resistor$ - alternate pin function. Must be either $mbed.pio.RES_PULLUP$ (default), $mbed.pio.RES_TRISTATE$, $mbed.pio.RES_PULLDOWN$."
      }
    },

  },
}

data_pt = data_en
