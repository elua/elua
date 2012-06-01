import os, sys, shutil, string
import platform as syspl
import glob, re, subprocess

# Helper: "normalize" a name to make it a suitable C macro name
def cnorm( name ):
  name = name.replace( '-', '' )
  name = name.replace( ' ', '' )
  return name.upper()

def gen_header( name, defines ):
  hname = os.path.join( os.getcwd(), "inc", name.lower() + ".h" )
  h = open( hname, "w" )
  h.write("// eLua " + name + " definition\n\n")
  h.write("#ifndef __" + name.upper() + "_H__\n")
  h.write("#define __" + name.upper() + "_H__\n\n")

  for key, value in defines.iteritems():
   h.write("#define   %-25s%-19s\n" % (key.upper(), value,))

  h.write("\n#endif\n")
  h.close()

# List of toolchains
toolchain_list = {
  'arm-gcc' : {
    'compile' : 'arm-elf-gcc',
    'link' : 'arm-elf-ld',
    'asm' : 'arm-elf-as',
    'bin' : 'arm-elf-objcopy',
    'size' : 'arm-elf-size',
    'cross_cpumode' : 'little',
    'cross_lua' : 'float_arm 64',
    'cross_lualong' : 'int 32'
  },
  'arm-eabi-gcc' : {
    'compile' : 'arm-eabi-gcc',
    'link' : 'arm-eabi-ld',
    'asm' : 'arm-eabi-as',
    'bin' : 'arm-eabi-objcopy',
    'size' : 'arm-eabi-size',
    'cross_cpumode' : 'little',
    'cross_lua' : 'float 64',
    'cross_lualong' : 'int 32'
  },
  'codesourcery' : {
    'compile' : 'arm-none-eabi-gcc',
    'link' : 'arm-none-eabi-ld',
    'asm' : 'arm-none-eabi-as',
    'bin' : 'arm-none-eabi-objcopy',
    'size' : 'arm-none-eabi-size',
    'cross_cpumode' : 'little',
    'cross_lua' : 'float 64',
    'cross_lualong' : 'int 32'
  },
  'avr32-gcc' : {
    'compile' : 'avr32-gcc',
    'link' : 'avr32-ld',
    'asm' : 'avr32-as',
    'bin' : 'avr32-objcopy',
    'size' : 'avr32-size',
    'cross_cpumode' : 'big',
    'cross_lua' : 'float 64',
    'cross_lualong' : 'int 32'
  },
  'avr32-unknown-none-gcc' : {
    'compile' : 'avr32-unknown-none-gcc',
    'link' : 'avr32-unknown-none-ld',
    'asm' : 'avr32-unknown-none-as',
    'bin' : 'avr32-unknown-none-objcopy',
    'size' : 'avr32-unknown-none-size',
    'cross_cpumode' : 'big',
    'cross_lua' : 'float 64',
    'cross_lualong' : 'int 32'
  },
  'i686-gcc' : {
    'compile' : 'i686-elf-gcc',
    'link' : 'i686-elf-ld',
    'asm' : 'nasm',
    'bin' : 'i686-elf-objcopy',
    'size' : 'i686-elf-size',
    'cross_cpumode' : 'little',
    'cross_lua' : 'float 64',
    'cross_lualong' : 'int 32'
  }
}

# Toolchain Aliases
toolchain_list['devkitarm'] = toolchain_list['arm-eabi-gcc']

# List of platform/CPU/toolchains combinations
# The first toolchain in the toolchains list is the default one
# (the one that will be used if none is specified)
platform_list = {
  'at91sam7x' : { 'cpus' : [ 'AT91SAM7X256', 'AT91SAM7X512' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ], 'big_endian': False },
  'lm3s' : { 'cpus' : [ 'LM3S1968', 'LM3S8962', 'LM3S6965', 'LM3S6918', 'LM3S9B92', 'LM3S9D92' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ], 'big_endian': False },
  'str9' : { 'cpus' : [ 'STR912FAW44' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ], 'big_endian': False },
  'i386' : { 'cpus' : [ 'I386' ], 'toolchains' : [ 'i686-gcc' ], 'big_endian': False },
  'sim' : { 'cpus' : [ 'LINUX' ], 'toolchains' : [ 'i686-gcc' ], 'big_endian': False },
  'lpc288x' : { 'cpus' : [ 'LPC2888' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ], 'big_endian': False },
  'str7' : { 'cpus' : [ 'STR711FR2' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ], 'big_endian': False },
  'stm32' : { 'cpus' : [ 'STM32F103ZE', 'STM32F103RE' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ], 'big_endian': False },
  'avr32' : { 'cpus' : [ 'AT32UC3A0512', 'AT32UC3A0256', 'AT32UC3A0128', 'AT32UC3B0256' ], 'toolchains' : [ 'avr32-gcc', 'avr32-unknown-none-gcc' ], 'big_endian': True },
  'lpc24xx' : { 'cpus' : [ 'LPC2468' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ], 'big_endian': False },
  'lpc17xx' : { 'cpus' : [ 'LPC1768' ], 'toolchains' : [ 'arm-gcc', 'codesourcery', 'devkitarm', 'arm-eabi-gcc' ], 'big_endian': False }
}

# List of board/CPU combinations
board_list = { 'SAM7-EX256' : [ 'AT91SAM7X256', 'AT91SAM7X512' ],
               'EK-LM3S1968' : [ 'LM3S1968' ],
               'EK-LM3S8962' : [ 'LM3S8962' ],
               'EK-LM3S6965' : [ 'LM3S6965' ],
               'EK-LM3S9B92' : [ 'LM3S9B92' ],
               'SOLDERCORE' : [ 'LM3S9D92' ],
               'STR9-COMSTICK' : [ 'STR912FAW44' ],
               'STR-E912' : [ 'STR912FAW44' ],
               'PC' : [ 'I386' ],
               'SIM' : [ 'LINUX' ],
               'LPC-H2888' : [ 'LPC2888' ],
               'MOD711' : [ 'STR711FR2' ],
               'STM3210E-EVAL' : [ 'STM32F103ZE' ],
               'ATEVK1100' : [ 'AT32UC3A0512' ],
               'ATEVK1101' : [ 'AT32UC3B0256' ],
               'ET-STM32' : [ 'STM32F103RE' ],
               'EAGLE-100' : [ 'LM3S6918' ],
               'ELUA-PUC' : ['LPC2468' ],
               'MBED' : ['LPC1768'],
               'MIZAR32' : [ 'AT32UC3A0256', 'AT32UC3A0512', 'AT32UC3A0128' ],
               'NETDUINO' : [ 'AT91SAM7X512' ],
            }

cpu_list = sum([board_list[i] for i in board_list],[])

comp = Environment( tools = [],
                    OBJSUFFIX = ".o",
                    PROGSUFFIX = ".elf",
                    ENV = os.environ,
                    CPPDEFINES = {} )

if comp['PLATFORM'] == 'win32':
  Tool('mingw')(comp)
else:
  Tool('default')(comp)

# Replacement for standard EnumVariable functionality to derive case from original list
class InsensitiveString(object):
  def __init__(self, s):
    self.s = s
  def __cmp__(self, other):
    return cmp(self.s.lower(), other.lower())

def _validator(key, val, env, vals):
  if not val in vals:
    raise SCons.Errors.UserError(
      'Invalid value for option %s: %s' % (key, val))

def MatchEnumVariable(key, help, default, allowed_values, map={}):
  help = '%s (%s)' % (help, string.join(allowed_values, '|'))

  validator = lambda key, val, env, vals=allowed_values: \
              _validator(key, InsensitiveString(val), env, vals)

  converter = lambda val, map=map: \
              map.get(val, allowed_values[allowed_values.index(InsensitiveString(val))])

  return (key, help, default, validator, converter)


# Add Configurable Variables
vars = Variables()

vars.AddVariables(
  MatchEnumVariable('bootloader',
                    'Build for bootloader usage, default is none.',
                    'none',
                    allowed_values = [ 'none', 'emblod' ] ),
  MatchEnumVariable('target',
                    'build "regular" float lua, 32 bit integer-only "lualong" or 64-bit integer-only "lualonglong"',
                    'lua',
                    allowed_values = [ 'lua', 'lualong', 'lualonglong' ] ),
  MatchEnumVariable('cpu',
                    'build for the specified CPU (board will be inferred, if possible)',
                    'auto',
                    allowed_values = cpu_list + [ 'auto' ] ),
  MatchEnumVariable('allocator',
                    'select memory allocator',
                    'auto',
                    allowed_values=[ 'newlib', 'multiple', 'simple', 'auto' ] ),
  MatchEnumVariable('board',
                    'selects board for target (cpu will be inferred)',
                    'auto',
                    allowed_values=board_list.keys() + [ 'auto' ] ),
  MatchEnumVariable('toolchain',
                    'specifies toolchain to use (auto=search for usable toolchain)',
                    'auto',
                    allowed_values=toolchain_list.keys() + [ 'auto' ] ),
  BoolVariable(     'optram',
                    'enables Lua Tiny RAM enhancements',
                    True ),
  MatchEnumVariable('boot',
                    'boot mode, standard will boot to shell, luarpc boots to an rpc server',
                    'standard',
                    allowed_values=[ 'standard' , 'luarpc' ] ),
  MatchEnumVariable('romfs',
                    'ROMFS compilation mode',
                    'verbatim',
                    allowed_values=[ 'verbatim' , 'compress', 'compile' ] ) )


vars.Update(comp)

if not GetOption( 'help' ):

  conf = Configure(comp)

  # Variants: board = <board>
  #           cpu = <cpuname>
  #           board = <board> cpu=<cpuname>
  if comp['board'] == 'auto' and comp['cpu'] == 'auto':
    print "Must specifiy board, cpu, or both"
    Exit( -1 )
  elif comp['board'] != 'auto' and comp['cpu'] != 'auto':
    # Check if the board, cpu pair is correct
    if not comp['cpu'] in board_list[ comp['board'] ]:
      print "Invalid CPU %s for board %s" % ( comp['cpu'], comp['board'] )
      Exit( -1 )
  elif comp['board'] != 'auto':
    # Find CPU
    comp['cpu'] = board_list[ comp['board'] ][ 0 ]
  else:
    # cpu = <cputype>
    # Find board name
    for b, v in board_list.items():
      if comp['cpu'] in v:
        comp['board'] = b
        break
    else:
      print "CPU %s not found" % comp['cpu']
      Exit( -1 )

  # Look for the given CPU in the list of platforms
  platform = None
  for p, v in platform_list.items():
    if comp['cpu'] in v[ 'cpus' ]:
      platform = p
      break
  else:
    print "Unknown CPU %s" % comp['cpu']
    print "List of accepted CPUs: "
    for p, v in platform_list.items():
      print " ", p, "-->",
      for cpu in v[ 'cpus' ]:
        print cpu,
      print
    Exit( -1 )

  # Check the toolchain
  if comp['toolchain'] != 'auto':
    if not comp['toolchain'] in platform_list[ platform ][ 'toolchains' ]:
      print "Invalid toolchain '%s' for CPU '%s'" % ( comp['toolchain'], comp['cpu'] )
      Exit( -1 )
    toolset = toolchain_list[ comp['toolchain'] ]
    comp[ 'CC' ] = toolset[ 'compile' ]
    comp[ 'AS' ] = toolset[ 'compile' ]
  else:
    # if 'auto' try to match a working toolchain with target
    usable_chains = [toolchain_list[ toolchain ][ 'compile' ] for toolchain in platform_list[ platform ]['toolchains']]
    comp['CC'] = comp.Detect( usable_chains )
    if comp['CC']:
        comp['toolchain'] =  platform_list[ platform ]['toolchains'][usable_chains.index(comp['CC'])]
        comp['AS'] = comp['CC']
        toolset = toolchain_list[ comp['toolchain'] ]
    else:
      print "Unable to find usable toolchain in your path."
      print "List of accepted toolchains (for %s):" % ( comp['cpu'] )
      print ', '.join(usable_chains)
      Exit( -1 )

    if not conf.CheckCC():
      print "Test compile failed with selected toolchain: %s" % (comp['toolchain'])
      Exit( -1 )

  # CPU/allocator mapping (if allocator not specified)
  if comp['allocator'] == 'auto':
    if comp['board'] in ['MIZAR32'] and comp['cpu'] in ['AT32UC3A0128']:
      comp['allocator'] = 'simple'
    elif comp['board'] in ['LPC-H2888', 'ATEVK1100', 'MIZAR32', 'MBED']:
      comp['allocator'] = 'multiple'
    else:
      comp['allocator'] = 'newlib'

  # Build the compilation command now
  compcmd = ''
  if comp['romfs'] == 'compile':
    if comp['target'] == 'lualonglong':
      print "Cross-compilation is not yet supported in 64-bit mode"
      Exit( -1 )
    if syspl.system() == 'Windows':
      suffix = '.exe'
    else:
      suffix = '.elf'
    # First check for luac.cross in the current directory
    if not os.path.isfile( "luac.cross" + suffix ):
      print "The eLua cross compiler was not found."
      print "Build it by running 'scons -f cross-lua.py'"
      Exit( -1 )
    compcmd = os.path.join( os.getcwd(), 'luac.cross%s -ccn %s -cce %s -o %%s -s %%s' % ( suffix, toolset[ 'cross_%s' % comp['target'] ], toolset[ 'cross_cpumode' ] ) )
  elif comp['romfs'] == 'compress':
    compcmd = 'lua luasrcdiet.lua --quiet --maximum --opt-comments --opt-whitespace --opt-emptylines --opt-eols --opt-strings --opt-numbers --opt-locals -o %s %s'

  # Determine build version
  try:
    elua_vers = subprocess.check_output(["git", "describe", "--always"]).strip()
    # If purely hexadecimal (no tag reference) prepend 'dev-'
    if re.match("^[0-9a-fA-F]+$",elua_vers):
      elua_vers = 'dev-' + elua_vers
    gen_header("git_version",{'elua_version': elua_vers, 'elua_str_version': "\"%s\"" % elua_vers } )
    conf.env.Append(CPPDEFINES = ['USE_GIT_REVISION'])
  except:
    print "WARNING: unable to determine version from repository"
    elua_vers = "unknown"


  # User report
  if not GetOption( 'clean' ):
    print
    print "*********************************"
    print "Compiling eLua ..."
    print "CPU:            ", comp['cpu']
    print "Board:          ", comp['board']
    print "Platform:       ", platform
    print "Allocator:      ", comp['allocator']
    print "Boot Mode:      ", comp['boot']
    print "Target:         ", comp['target']
    print "Toolchain:      ", comp['toolchain']
    print "ROMFS mode:     ", comp['romfs']
    print "Version:        ", elua_vers
    print "*********************************"
    print

  output = 'elua_' + comp['target'] + '_' + comp['cpu'].lower()

  comp.Append(CPPDEFINES = { 'ELUA_CPU' : comp['cpu'],
                             'ELUA_BOARD' : comp['board'],
                             'ELUA_PLATFORM' : platform.upper() } )
  comp.Append(CPPDEFINES = {'__BUFSIZ__' : 128})

  # Also make the above into direct defines (to use in conditional C code)
  conf.env.Append(CPPDEFINES = ["ELUA_CPU_" + cnorm( comp['cpu'] ), "ELUA_BOARD_" + cnorm( comp['board'] ), "ELUA_PLATFORM_" +  cnorm( platform )])

  if comp['allocator'] == 'multiple':
     conf.env.Append(CPPDEFINES = ['USE_MULTIPLE_ALLOCATOR'])
  elif comp['allocator'] == 'simple':
     conf.env.Append(CPPDEFINES = ['USE_SIMPLE_ALLOCATOR'])

  if comp['boot'] == 'luarpc':
    conf.env.Append(CPPDEFINES = ['ELUA_BOOT_RPC'])

  # Special macro definitions for the SYM target
  if platform == 'sim':
    conf.env.Append(CPPDEFINES = ['ELUA_SIMULATOR',"ELUA_SIM_" + cnorm( comp['cpu'] ) ] )

  # Lua source files and include path
  lua_files = """lapi.c lcode.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c
    lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c
    ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c lua.c lrotable.c legc.c"""

  lua_full_files = " " + " ".join( [ "src/lua/%s" % name for name in lua_files.split() ] )

  comp.Append(CPPPATH = ['inc', 'inc/newlib',  'inc/remotefs', 'src/platform', 'src/lua'])
  if comp['target'] == 'lualong' or comp['target'] == 'lualonglong':
    conf.env.Append(CPPDEFINES = ['LUA_NUMBER_INTEGRAL'])
  if comp['target'] == 'lualonglong':
    conf.env.Append(CPPDEFINES = ['LUA_INTEGRAL_LONGLONG'])
  if comp['target'] != 'lualong' and comp['target'] != 'lualonglong':
    conf.env.Append(CPPDEFINES = ['LUA_PACK_VALUE'])
  if platform_list[platform]['big_endian']:
    conf.env.Append(CPPDEFINES = ['ELUA_ENDIAN_BIG'])
  else:
    conf.env.Append(CPPDEFINES = ['ELUA_ENDIAN_LITTLE'])
  conf.env.Append(CPPPATH = ['src/modules', 'src/platform/%s' % platform])
  conf.env.Append(CPPDEFINES = {"LUA_OPTIMIZE_MEMORY" : ( comp['optram'] != 0 and 2 or 0 ) } )

  # Additional libraries
  local_libs = ''

  # Application files
  app_files = """ src/main.c src/romfs.c src/semifs.c src/xmodem.c src/shell.c src/term.c src/common.c src/common_tmr.c src/buf.c src/elua_adc.c src/dlmalloc.c
                  src/salloc.c src/luarpc_elua_uart.c src/elua_int.c src/linenoise.c src/common_uart.c src/eluarpc.c """

  # Newlib related files
  newlib_files = " src/newlib/devman.c src/newlib/stubs.c src/newlib/genstd.c src/newlib/stdtcp.c"

  # UIP files
  uip_files = "uip_arp.c uip.c uiplib.c dhcpc.c psock.c resolv.c uip-neighbor.c"
  uip_files = " src/elua_uip.c " + " ".join( [ "src/uip/%s" % name for name in uip_files.split() ] )
  comp.Append(CPPPATH = ['src/uip'])

  # FatFs files
  app_files = app_files + "src/elua_mmc.c src/mmcfs.c src/fatfs/ff.c src/fatfs/ccsbcs.c "
  comp.Append(CPPPATH = ['src/fatfs'])

  # Lua module files
  module_names = "pio.c spi.c tmr.c pd.c uart.c term.c pwm.c lpack.c bit.c net.c cpu.c adc.c can.c luarpc.c bitarray.c elua.c i2c.c"
  module_files = " " + " ".join( [ "src/modules/%s" % name for name in module_names.split() ] )

  # Remote file system files
  rfs_names = "remotefs.c client.c elua_os_io.c elua_rfs.c"
  rfs_files = " " + " ".join( [ "src/remotefs/%s" % name for name in rfs_names.split() ] )

  # Optimizer flags (speed or size)
  comp.Append(CCFLAGS = ['-Os','-fomit-frame-pointer'])
  #opt += " -ffreestanding"
  #opt += " -fconserve-stack" # conserve stack at potential speed cost, >=GCC4.4

  # Toolset data (filled by each platform in part)
  tools = {}

  # We get platform-specific data by executing the platform script
  execfile( "src/platform/%s/conf.py" % platform )

  # Complete file list
  source_files = Split( app_files + specific_files + newlib_files + uip_files + lua_full_files + module_files + rfs_files )

  comp = conf.Finish()

  romfs_exclude = [ '.DS_Store' ]

  # Make ROM File System first
  if not GetOption( 'clean' ):
    print "Building ROM File System..."
    flist = []
    os.chdir( "romfs" );
    for sample in glob.glob("*"):
      if sample not in romfs_exclude:
        flist += [ sample ]
    os.chdir( ".." )
    import mkfs
    mkfs.mkfs( "romfs", "romfiles", flist, comp['romfs'], compcmd )
    print
    if os.path.exists( "inc/romfiles.h" ):
      os.remove( "inc/romfiles.h" )
    shutil.move( "romfiles.h", "inc/" )
    if os.path.exists( "src/fs.o" ):
      os.remove( "src/fs.o" )

  # comp.TargetSignatures( 'content' )
  # comp.SourceSignatures( 'MD5' )
  comp[ 'INCPREFIX' ] = "-I"
  Default( comp.Program( target = output, source = source_files ) )
  Decider( 'MD5-timestamp' )

  # Programming target
  prog = Environment( BUILDERS = { 'program' : Builder( action = Action ( tools[ platform ][ 'progfunc' ] ) ) }, ENV = os.environ )
  prog.program( "prog", output + ".elf" )

Help(vars.GenerateHelpText(comp))
