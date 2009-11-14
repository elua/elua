-- eLua reference manual - SPI module

data_en = 
{

  -- Title
  title = "eLua reference manual - LuaRPC module",

  -- Menu name
  menu_name = "rpc",

  -- Overview
  overview = [[LuaRPC allows one to remotely control a lua state running on another machine.]],

  -- Functions
  funcs = 
  {
    { sig = "handle = #rpc.connect#( transport_identifiers )",
      desc = "Initiate connection from client to server.",
      args = 
      {
        "$transport_identifiers$ - identifiers for channel connecting to server (specific to selected transport)",
      },
      ret = "$handle$ - used to interact with the remote Lua state."
    },

    { sig = "#rpc.close#( handle )",
      desc = "Close an active RPC session.",
      args = "$handle$ - handle associated with the connection.",
    },

    { sig = "#rpc.server#( transport_identifiers )",
      desc = "Start a blocking/captive RPC server, which will wait for incoming connections.",
      args = "$transport_identifiers$ - identifiers for the transport channel to listen on (specific to selected transport).",
    },

    { sig = "#rpc.on_error#( err_handler )",
      desc = "Define client-side error handler to deal with server-side error responses.",
      args = "$err_handler$ - function to handle error messages. string error messages may be passed to this function.",
    },

    { sig = "server_handle = #rpc.listen#( transport_identifiers )",
      desc = "Open a listener on transport and await incoming connections.",
      args = "$transport_identifiers$ - identifiers for the transport channel to listen on (specific to selected transport).",
      ret = "server handle to use with @#rpc.peek@rpc.peek@ and @#rpc.dispatch@rpc.dispatch@"
    },
    
    { sig = "data_available = #rpc.peek#( server_handle )",
      desc = "Check if data are available to read on transport.",
      args = "$server_handle$ - handle to refer to server session, created by @#rpc.listen@rpc.listen@",
      ret = "$data_available$ - 1 if data are available, 0 if data are unavailable"
    },
    
    { sig = "#rpc.dispatch#( server_handle )",
      desc = "Read transport and handle incoming command.",
      args = "$server_handle$ - handle to refer to server session, created by @#rpc.listen@rpc.listen@",
    },
   
  },

}

data_pt = data_en

