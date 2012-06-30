-- eLua components description

module( ..., package.seeall )

local sf = string.format
local ct = require "constants"
local at = require "attributes"
local gen = require "generators"

-------------------------------------------------------------------------------
-- Consistency checking

-- Helper: shell configuration consistency checker
-- Returns 'true' if consistent, (false, errmsg) otherwise
local function shell_confcheck( eldesc, vals )
  if vals._SHELL_TRANSPORT.value == 'serial' then
    -- Check for required CON_UART_ID/CON_TIMER_ID/CON_UART_SPEED/CON_UART_FLOW
    if not vals.CON_UART_ID then
      return false, "required attribute 'uart' not specified for element 'shell' in section 'components'"
    elseif not vals.CON_UART_SPEED then
      return false, "required attribute 'speed' not specified for element 'shell' in section 'components'"
    end
  elseif vals._SHELL_TRANSPORT.value == 'tcpip' then
    if not enabled[ 'tcpip' ] then
      return false, "shell over TCP/IP can't be enabled if the 'tcpip' component is not enabled in section 'components'"
    end
  else
    return false, sf( "invalid value '%s' for attribute 'transport' of element 'shell' in section 'components'", vals._SHELL_TRANSPORT.value )
  end
  return true
end

-------------------------------------------------------------------------------
-- Specific generators

-- Generator for the shell component
local function shell_gen( desc, conf, generated )
  generated._SHELL_TRANSPORT = true
  local gstr = ''
  local shtype = conf._SHELL_TRANSPORT.value
  if shtype == 'serial' then
    gstr = gstr .. gen.simple_gen( "CON_UART_ID", conf, generated )
    gstr = gstr .. gen.simple_gen( "CON_UART_SPEED", conf, generated )
    gstr = gstr .. gen.simple_gen( "CON_TIMER_ID", conf, generated )
    gstr = gstr .. gen.simple_gen( "CON_FLOW_TYPE", conf, generated )
    gstr = gstr .. gen.simple_gen( "CON_BUF_SIZE", conf, generated )
  else
    error "TODO: implement shell over TCP/IP generator"
  end
  return gstr
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
      uart = at.int_attr( 'CON_UART_ID' ),
      speed = at.int_attr( 'CON_UART_SPEED' ),
      timer = at.timer_attr( 'CON_TIMER_ID' ),
      flow = at.flow_control_attr( 'CON_FLOW_TYPE' ),
      buf_size = at.make_optional( at.int_log2_attr( 'CON_BUF_SIZE' ) )
    }
  }
  -- TCP/IP console
  components.tcpipcon = { macro = 'BUILD_CON_TCP' }
  -- UART buffering
  -- Not really a component, not quite a config ... Implementation wise,
  -- it is easier to declare it as a component
  components.uart_buffers = { macro = 'BUF_ENABLE_UART' }
  -- XMODEM
  components.xmodem = {
    macro = 'BUILD_XMODEM',
    attrs = {
      uart = at.int_attr( 'CON_UART_ID' ),
      speed = at.int_attr( 'CON_UART_SPEED' ),
      timer = at.timer_attr( 'CON_TIMER_ID' ),
      flow = at.flow_control_attr( 'CON_FLOW_TYPE'),
      buf_size = at.make_optional( at.int_log2_attr( 'CON_BUF_SIZE' ) )
    }
  }
  -- Shell
  components.shell = {
    macro = 'BUILD_SHELL',
    confcheck = shell_confcheck,
    gen = shell_gen,
    attrs = {
      transport = at.choice_attr( '_SHELL_TRANSPORT', { 'serial', 'tcpip' }, 'serial' ),
      uart = at.int_attr( 'CON_UART_ID' ),
      speed = at.int_attr( 'CON_UART_SPEED' ),
      timer = at.timer_attr( 'CON_TIMER_ID' ),
      flow = at.flow_control_attr( 'CON_FLOW_TYPE' ),
      buf_size = at.make_optional( at.int_log2_attr( 'CON_BUF_SIZE' ) )
    }
  }
  -- Term
  components.term = {
    macro = 'BUILD_TERM',
    attrs = {
      uart = at.int_attr( 'CON_UART_ID' ),
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
    }
  }
  -- Linenoise
  components.linenoise = { 
    macro = 'BUILD_LINENOISE',
    attrs = {
      shell_lines = at.int_attr( 'LINENOISE_HISTORY_SIZE_SHELL' ),
      lua_lines = at.int_attr( 'LINENOISE_HISTORY_SIZE_LUA' ),
      autosave_file = at.make_optional( at.string_attr( 'LINENOISE_AUTOSAVE_FNAME', '', 32 ) )
    }
  }
  -- RFS
  components.rfs = {
    macro = 'BUILD_RFS',
    attrs = {
      uart = at.int_attr( 'RFS_UART_ID' ),
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
      port = at.int_attr( 'MMCFS_CS_PORT' ),
      pin = at.int_attr( 'MMCFS_CS_PIN' ),
      spi = at.int_attr( 'MMCFS_SPI_NUM' )
    }
  }
  -- RPC
  -- (values are optional, since they are required only if booting in RPC mode)
  components.rpc = {
    macro = 'BUILD_RPC',
    attrs = {
      uart = at.make_optional( at.int_attr( 'RPC_UART_ID' ) ),
      speed = at.make_optional( at.int_attr( 'RPC_UART_SPEED' ) ),
      timer = at.make_optional( at.timer_attr( 'RPC_TIMER_ID' ) )
    }
  }
  -- ROMFS
  components.romfs = { macro = 'BUILD_ROMFS' }
  -- WOFS
  components.wofs = { macro = "BUILD_WOFS" }
  -- All done
  return components
end

