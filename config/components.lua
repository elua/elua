-- eLua components description

module( ..., package.seeall )
local at = require "attributes"
local gen = require "generators"
local sf = string.format

-------------------------------------------------------------------------------
-- Sermux support

local function sermux_auxcheck( eldesc, data, enabled )
  local v = data.SERMUX_BUFFER_SIZES.value
  if #v < 2 then
    return false, sf( "array 'buf_sizes' of element 'sermux' in section 'components' must have at least 2 elements" )
  end
  return true
end

local function sermux_auxgen( eldesc, data, generated )
  local v = data.SERMUX_BUFFER_SIZES.value
  return gen.print_define( 'SERMUX_NUM_VUART', #v )
end

-------------------------------------------------------------------------------
-- MMCFS support

local function mmcfs_auxcheck( eldesc, data, enabled )
  local ports, pins, spis = data.MMCFS_CS_PORT.value, data.MMCFS_CS_PIN.value, data.MMCFS_SPI_NUM.value
  if #ports ~= #pins or #pins ~= #spis then
    return false, "non-equal sizes for values of 'cs_port', 'cs_pin' and 'spi' of element 'mmcfs' in secetion 'components'"
  end
  return true
end

local function mmcfs_gen( eldesc, data, generated )
  local ports, pins, spis = data.MMCFS_CS_PORT.value, data.MMCFS_CS_PIN.value, data.MMCFS_SPI_NUM.value
  local data = ''
  if #ports == 1 then -- single card
    data = data .. gen.print_define( 'MMCFS_CS_PORT', ports[ 1 ] )
    data = data .. gen.print_define( 'MMCFS_CS_PIN', pins[ 1 ] )
    data = data .. gen.print_define( 'MMCFS_SPI_NUM', spis[ 1 ] )
  else -- multiple cards
    data = data .. gen.print_define( 'MMCFS_CS_PORT_ARRAY', ports )
    data = data .. gen.print_define( 'MMCFS_CS_PIN_ARRAY', pins )
    data = data .. gen.print_define( 'MMCFS_SPI_NUM_ARRAY', spis )
  end
  return data
end

-------------------------------------------------------------------------------
-- Shell support

-- Mapping between shell commands and their corresponding functions in shell.c
local shell_cmd_map = {
  help = "shell_help", lua = "shell_lua", recv = "shell_recv", ver = "shell_ver",
  exit = "NULL", ls = "shell_ls", dir = "shell_ls", cat = "shell_cat",
  type = "shell_cat", cp = "shell_cp", wofmt = "shell_wofmt", mkdir = "shell_mkdir",
  rm = "shell_adv_rm", mv = "shell_adv_mv"
}

local function shell_gen( eldesc, data, generated )
  local cmds, advv = data._DUMMY_SHELL_DATA.value, data.BUILD_ADVANCED_SHELL.value
  local data = '#define SHELL_COMMAND_LIST\\\n'
  for i, c in pairs( cmds ) do
      data = data .. sf('  { "%s", %s },\\\n', c, shell_cmd_map[c] )
  end
  data = data .. "  { NULL, NULL }\n"
  if advv == 1 then data = data .. "#define BUILD_ADVANCED_SHELL\n" end
  return data
end

-------------------------------------------------------------------------------
-- Return a CDC component
-- This should be included by each backend that supports USB UARTs

function cdc_uart()
  return {
    macro = "BUILD_USB_CDC",
    attrs = {
      buf_size = at.make_optional( at.int_log2_attr( 'CDC_BUF_SIZE', 2 ) )
    }
  }
end

-------------------------------------------------------------------------------
-- Public interface

-- Build all components needed by eLua, save them in the "components" table
function init()
  local components = {}

  -- Serial console
  components.sercon = {
    macro = 'BUILD_CON_GENERIC',
    attrs = {
      uart = at.uart_attr( 'CON_UART_ID' ),
      speed = at.int_attr( 'CON_UART_SPEED' ),
      timer = at.timer_attr( 'CON_TIMER_ID' ),
      flow = at.flow_control_attr( 'CON_FLOW_TYPE' ),
      buf_size = at.make_optional( at.int_log2_attr( 'CON_BUF_SIZE' ) )
    }
  }
  -- TCP/IP console
  components.tcpipcon = { macro = 'BUILD_CON_TCP', needs = 'tcpip' }
  -- UART buffering
  -- Not really a component, not quite a config ... Implementation wise,
  -- it is easier to declare it as a component
  components.uart_buffers = { macro = 'BUF_ENABLE_UART' }
  -- XMODEM
  components.xmodem = {
    macro = 'BUILD_XMODEM',
    attrs = {
      uart = at.uart_attr( 'CON_UART_ID' ),
      speed = at.int_attr( 'CON_UART_SPEED' ),
      timer = at.timer_attr( 'CON_TIMER_ID' ),
      flow = at.flow_control_attr( 'CON_FLOW_TYPE'),
      buf_size = at.make_optional( at.int_log2_attr( 'CON_BUF_SIZE' ) )
    }
  }
  -- Shell
  local shell_cmds_attr = at.choice_attr( '_DUMMY_SHELL_DATA', utils.table_keys( shell_cmd_map ), utils.table_keys( shell_cmd_map ))
  components.shell = { macro = 'BUILD_SHELL',
    attrs = {
      commands = at.array_of( shell_cmds_attr, false ),
      advanced = at.bool_attr( 'BUILD_ADVANCED_SHELL', false )
    },
    gen = shell_gen
  }
  -- Term
  components.term = {
    macro = 'BUILD_TERM',
    attrs = {
      uart = at.uart_attr( 'CON_UART_ID' ),
      speed = at.int_attr( 'CON_UART_SPEED' ),
      timer = at.timer_attr( 'CON_TIMER_ID' ),
      flow = at.flow_control_attr( 'CON_FLOW_TYPE' ),
      buf_size = at.make_optional( at.int_log2_attr( 'CON_BUF_SIZE' ) ),
      lines = at.int_attr( 'TERM_LINES' ),
      cols = at.int_attr( 'TERM_COLS' )
    }
  }
  -- C interrupt support
  components.cints = { macro = 'BUILD_C_INT_HANDLERS' }
  -- Lua interrupt support
  components.luaints = {
    macro = 'BUILD_LUA_INT_HANDLERS',
    attrs = {
      queue_size = at.int_log2_attr( 'PLATFORM_INT_QUEUE_LOG_SIZE', nil, nil, 5 )
    },
    needs = 'cints'
  }
  -- Linenoise
  components.linenoise = {
    macro = 'BUILD_LINENOISE',
    attrs = {
      shell_lines = at.int_attr( 'LINENOISE_HISTORY_SIZE_SHELL' ),
      lua_lines = at.int_attr( 'LINENOISE_HISTORY_SIZE_LUA' ),
      autosave_file = at.make_optional( at.string_attr( 'LINENOISE_AUTOSAVE_FNAME', 32 ) )
    }
  }
  -- RFS
  components.rfs = {
    macro = 'BUILD_RFS',
    attrs = {
      uart = at.uart_attr( 'RFS_UART_ID' ),
      speed = at.int_attr( 'RFS_UART_SPEED' ),
      timer = at.timer_attr( 'RFS_TIMER_ID' ),
      flow = at.flow_control_attr( 'RFS_FLOW_TYPE' ),
      buf_size = at.int_log2_attr( 'RFS_BUFFER_SIZE', nil, nil, 9 ),
      timeout = at.int_attr( 'RFS_TIMEOUT', nil, nil, 100000 )
    }
  }
  -- MMCFS
  components.mmcfs = {
    macro = 'BUILD_MMCFS',
    auxcheck = mmcfs_auxcheck,
    gen = mmcfs_gen,
    attrs = {
      cs_port = at.array_of( at.int_attr( 'MMCFS_CS_PORT' ), true ),
      cs_pin = at.array_of( at.int_attr( 'MMCFS_CS_PIN' ), true ),
      spi = at.array_of( at.int_attr( 'MMCFS_SPI_NUM' ), true )
    }
  }
  -- RPC
  -- (values are optional, since they are required only if booting in RPC mode)
  components.rpc = {
    macro = 'BUILD_RPC',
    attrs = {
      uart = at.make_optional( at.uart_attr( 'RPC_UART_ID' ) ),
      speed = at.make_optional( at.int_attr( 'RPC_UART_SPEED' ) ),
      timer = at.make_optional( at.timer_attr( 'RPC_TIMER_ID' ) )
    }
  }
  -- TCP/IP
  components.tcpip = {
    macro = 'BUILD_UIP',
    attrs = {
      ip = at.ip_attr( 'ELUA_CONF_IPADDR' ),
      netmask = at.ip_attr( 'ELUA_CONF_NETMASK' ),
      gw = at.ip_attr( 'ELUA_CONF_DEFGW' ),
      dns = at.ip_attr( 'ELUA_CONF_DNS' )
    }
  }
  -- Serial multiplexer
  components.sermux = {
    macro = 'BUILD_SERMUX',
    auxgen = sermux_auxgen,
    auxcheck = sermux_auxcheck,
    attrs = {
      uart = at.uart_attr( 'SERMUX_PHYS_ID' ),
      speed = at.int_attr( 'SERMUX_PHYS_SPEED' ),
      flow = at.flow_control_attr( 'SERMUX_FLOW_TYPE' ),
      buf_sizes = at.array_of( at.int_log2_attr( 'SERMUX_BUFFER_SIZES' ) )
    }
  }
  -- ADC
  components.adc = {
    macro = 'BUILD_ADC',
    attrs = {
      buf_size = at.make_optional( at.int_log2_attr( 'ADC_BUF_SIZE' ) )
    }
  }
  -- DNS client
  components.dns = { macro = 'BUILD_DNS', needs = 'tcpip' }
  -- DHCP client
  components.dhcp = { macro = 'BUILD_DHCPC', needs = 'tcpip' }
  -- ROMFS
  components.romfs = { macro = 'BUILD_ROMFS' }
  -- WOFS
  components.wofs = { macro = "BUILD_WOFS" }
  -- NIFFS
  components.niffs = {
    macro = "BUILD_NIFFS",
    attrs = {
      linear_area = at.make_optional( at.bool_attr( 'NIFFS_LINEAR_AREA' ) )
    }
  }

  -- All done
  return components
end

