-- eLua reference manual - uart module

data_en = 
{

  -- Title
  title = "eLua reference manual - UART module",

  -- Menu name
  menu_name = "uart",

  -- Overview
  overview = [[This module contains functions for accessing the serial ports (UARTs) of the eLua CPU.]],

  -- Functions
  funcs = 
  {
    { sig = "baud = #uart.setup#( id, baud, databits, parity, stopbits )",
      desc = "Setup the serial port",
      args = 
      {
        "$id$ - the ID of the serial port",
        "$baud$ - serial baud rate",
        "$databits$ - number of data bits",
        "$parity$ - parity type, can be either $uart.PAR_EVEN$, $uart.PAR_ODD$ or $uart.PAR_NONE$",
        [[$stopbits$ - the number of stop bits, can be either $uart.STOP_1$ (for 1 stop bit), $uart.STOP_1_5$ (for 1.5 stop bits) or $uart.STOP_2$ 
(for 2 stop bits)]]
      },
      ret = "The actual baud rate set on the serial port. Depending on the hardware, this might have a different value than the $baud$ parameter"
    },

    { sig = "#uart.write#( id, data1, [data2], ..., [datan] )",
      desc = [[Write one or more strings or numbers to the serial port. If writing a number, its value must be between 0 and 255.]],
      args = 
      {
        "$id$ - the ID of the serial port.",
        "$data1$ - the first string/number to write.",
        "$data2 (optional)$ - the second string/number to write.",
        "$datan (optional)$ - the %n%-th string/number to write."
      }
    },

    { sig = "str = #uart.getchar#( id, [timeout], [timer_id] )",
      desc = "Read a single character from the serial port",
      args = 
      {
        "$id$ - the ID of the serial port",
        [[$timeout (optional)$ - timeout of the receive operation, can be either $uart.NO_TIMEOUT$ or 0 for non-blocking operation, $uart.INF_TIMEOUT$ for 
blocking operation, or a positive number that specifies the timeout in microseconds (in this case, the $timer_id$ parameter is also required). The default
value of this argument is $uart.INF_TIMEOUT$]],
        [[$timer_id (optional)$ - the ID of the timer for the receive operation, needed if the $timeout$ parameter specifies an actual timeout (that is,
$timeout$ is neither $uart.NO_TIMEOUT$, nor $uart.INF_TIMEOUT$).]]
      },
      ret = "The character read from the serial port as a string, or the empty string it timeout occured while waiting for the character."
    },

    { sig = "str = #uart.read#( id, format, [timeout], [timer_id] )",
      desc = "Reads one or more characters from the serial port according to a format specifier",
      args = 
      {
        "$id$ - the ID of the serial port",
        [[$format$ - format of data to read. This can be either:
<ul>
  <li>$'*l'$ - read until an end of line character (a $\n$) is found (the $\n$ is not returned) or a timeout occurs.</li>
  <li>$'*n'$ - read an integer. The integer can optionally have a sign. Reading continues until the first non-digit character is detected or a timeout occurs. This is the only case in which $read$ returns a number instead of an integer.</li>
  <li>$'*s'$ - read until a spacing character (like a space or a TAB) is found (the spacing character is not returned) or a timeout occurs.</li>
  <li>$a positive number$ - read at most this many characters before returning (reading can stop earlier if a timeout occurs).</li>
</ul>]],
        [[$timeout (optional)$ - timeout of the receive operation, can be either $uart.NO_TIMEOUT$ or 0 for non-blocking operation, $uart.INF_TIMEOUT$ for 
blocking operation, or a positive number that specifies the inter-char timeout in microseconds (in this case, the $timer_id$ parameter is also required). The default value of this argument is $uart.INF_TIMEOUT$]],
        [[$timer_id (optional)$ - the ID of the timer for the receive operation, needed if the $timeout$ parameter specifies an actual timeout (that is,
$timeout$ is neither $uart.NO_TIMEOUT$, nor $uart.INF_TIMEOUT$).]]
      },
      ret = [[The data read from the serial port as a string (or as a number if $format$ is $'*n'$). If a timeout occures, only the data read before the timeout is returned. If the function times out while trying to read the first character, the empty string is returned]]
    }
   
  },

}

