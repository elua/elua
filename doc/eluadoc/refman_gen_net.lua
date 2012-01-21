-- eLua reference manual - net module

data_en = 
{

  -- Title
  title = "eLua reference manual - net module",

  -- Menu name
  menu_name = "net",

  -- Overview
  overview = [[This module contains functions for accessing a TCP/IP network from eLua. It can be enabled only if networking support is also enabled
  (see @building.html@building@ for details).</p>
  <p><span class="warning">NOTE:</span> TCP/IP support is $experimental$ in eLua. While functional, it's still slow and suffers from a number of
  other issues. It will most likely change a lot in the future, so expect major changes to this module as well.</p>
  <p><span class="warning">NOTE:</span> currently, only TCP sockets are supported by eLua.]],

  -- Structures
  structures =
  {
    { text = [[// eLua net error codes
enum
{
  ELUA_NET_ERR_OK = 0,            // exported as $net.ERR_OK$
  ELUA_NET_ERR_TIMEDOUT,          // exported as $net.ERR_TIMEDOUT$
  ELUA_NET_ERR_CLOSED,            // exported as $net.ERR_CLOSED$
  ELUA_NET_ERR_ABORTED,           // exported as $net.ERR_ABORTED$
  ELUA_NET_ERR_OVERFLOW           // exported as $net.ERR_OVERFLOW$
};]],
      name = "Error codes",
      desc = "These are the error codes defined by the eLua networking layer and they are also returned by a number of functions in this module.",
    }
  },

  -- Functions
  funcs = 
  {
    { sig = "ip = #net.packip#( ip1, ip2, ip3, ip4 )",
      desc = [[Returns an internal representation of an IP address that can be used with all function from the $net$ module that expect an IP address
argument. The IP is considered to be in the format %ip1.ip2.ip3.ip4%.]],
      args = 
      {
        "$ip1$ - the first part of the IP address.",
        "$ip2$ - the second part of the IP address.",
        "$ip3$ - the third part of the IP address.",
        "$ip4$ - the fourth part of the IP address."
      },
      ret = "An integer that encodes the IP address in an internal format."
    },

     { sig = "ip = #net.packip#( 'ip' )",
      desc = [[Returns an internal representation of an IP address that can be used with all function from the $net$ module that expect an IP address
argument. The IP is given as a string.]],
      args = "$ip$ - the IP address in string format.",
      ret = "An integer that encodes the IP address in an internal format."
    },

    { sig = "ip1, ip2, ip3, ip4 = #net.unpackip#( ip, '*n' )",
      desc = "Returns an unpacked representation of an IP address encoded by @#net.packip@net.packip@.",
      args = "$ip$ - the encoded IP address.",
      ret = 
      {
        "$ip1$ - the first part of the IP address.",
        "$ip2$ - the second part of the IP address.",
        "$ip3$ - the third part of the IP address.",
        "$ip4$ - the fourth part of the IP address."
      }
    },
   
    { sig = "ip = #net.unpackip#( ip, '*s' )",
      desc = "Returns an unpacked representation of an IP address encoded by @#net.packip@net.packip@.",
      args = "$ip$ - the encoded IP address.",
      ret = "The IP address in string format."
    },


    { sig = "ip = #net.lookup#( hostname )",
      desc = "Does a DNS lookup.",
      args = "$hostname$ - the name of the computer.",
      ret = "The IP address of the computer."
    },

    { sig = "socket = #net.socket#( type )",
      desc = "Create a socket for TCP/IP communication.",
      args = [[$type$ - can be either $net.SOCK_STREAM$ for TCP sockets or $net.SOCK_DGRAM$ for UDP sockets (<span class="warning">not yet supported</span>).]],
      ret = "The socket that will be used in subsequent operations."
    },

    { sig = "res = #net.close#( socket )",
      desc = "Close a socket.",
      args = "$socket$ - the socket to close.",
      ret = "An error code, as defined @#error_codes@here@."
    },

    { sig = "err = #net.connect#( sock, ip, port )",
      desc = "Connect a socket to a remote system.",
      args = 
      {
        "$sock$ - a socket obtained from @#net.socket@net.socket@.",
        "$ip$ - the IP address obtained from @#net.packip@net.packip@.",
        "$port$ - the port to connecto to."
      },
      ret = "$err$ - the error code, as defined @#error_codes@here@."
    },

    { sig = "socket, remoteip, err = #net.accept#( port, [timer_id, timeout] )",
      desc = "Accept a connection from a remote system with an optional timeout.",
      args =
      {
        "$port$ - the port to wait for connections from the remote system.",
        [[$timer_id (optional)$ - the ID of the timer used for measuring the timeout. Use $nil$ or $tmr.SYS_TIMER$ to specify the @arch_platform_timers.html#the_system_timer@system timer@.]],
       [[$timeout (optional)$ - timeout of the operation, can be either $net.NO_TIMEOUT$ or 0 for non-blocking operation, $net.INF_TIMEOUT$ for 
blocking operation, or a positive number that specifies the timeout in microseconds. The default value of this argument is $unet.INF_TIMEOUT$.]],

      },
      ret =
      {
        "$socket$ - the socket created after accepting the remote connection.",
        "$remoteip$ - the IP of the remote system.",
        "$err$ - an error code, as defined @#error_codes@here@."
      }
    },

    { sig = "res, err = #net.send#( sock, str )",
      desc = "Send data to a socket.",
      args = 
      {
        "$sock$ - the socket.",
        "$str$ - the data to send."
      },
      ret = 
      {
        "$res$ - the number of bytes actually sent or -1 for error.",
        "$err$ - the error code, as defined @#error_codes@here@."
      }
    },

    { sig = "res, err = #net.recv#( sock, format, [timer_id, timeout] )",
      desc = "Read data from a socket.",
      args = 
      {
        "$sock$ - the socket.",
        [[$format$ - how to read the data. This can be either:
<ul>
  <li>$"*l"$: read a line (until the next '\n' character).</li>
  <li>$an integer$: read up to that many bytes.</li>
</ul>]],
        [[$timer_id (optional)$ - the ID of the timer used for measuring the timeout. Use $nil$ or $tmr.SYS_TIMER$ to specify the @arch_platform_timers.html#the_system_timer@system timer@.]],
        [[$timeout (optional)$ - timeout of the operation, can be either $net.NO_TIMEOUT$ or 0 for non-blocking operation, $net.INF_TIMEOUT$ for 
blocking operation, or a positive number that specifies the timeout in microseconds. The default value of this argument is $net.INF_TIMEOUT$.]],
      },
      ret =
      {
        "$res$ - the number of bytes read.",
        "$err$ - the error code, as defined @#error_codes@here@."
      }
    }
  },
}

