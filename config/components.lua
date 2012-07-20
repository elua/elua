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
-- Return a CDC component
-- This should be included by each backend that supports USB UARTs

function cdc_uart()
  return { macro = "BUILD_USB_CDC" }
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
  components.shell = { macro = 'BUILD_SHELL' }
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
    attrs = {
      cs_port = at.int_attr( 'MMCFS_CS_PORT' ),
      cs_pin = at.int_attr( 'MMCFS_CS_PIN' ),
      spi = at.int_attr( 'MMCFS_SPI_NUM' )
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
      buf_size = at.make_optional( at.int_log2_attr( 'ADC_BUF_SIZE' ) ),
      first_timer = at.make_optional( at.int_attr( 'ADC_TIMER_FIRST_ID' ) ),
      num_timers = at.make_optional( at.int_attr( 'ADC_NUM_TIMERS' ) )
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
  -- All done
  return components
end

