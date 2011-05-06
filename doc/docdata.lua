-- The eLua documentation structure (main menu) is defined here.

--[[
The menu is defined by an array of { name, link, submenu, title } arrays

'name' can be another array. Names must be the same for all languages, in the same order 
as defined in the "languages" from 'buildall.lua'. If 'name' is a string instead
of a table, it will be used in all languages.

'link' is the relative link corresponding to the menu entry.

'submenu' is optional. If specified, it is an array of arrays, just like the main
menu.

'title' is the title of the HTML page associated with the menu entry (specified as 
an array in different languages or as a string, just like 'name'). If not given, 
it's the same as 'name'.

If 'link' is nil, the entry defines a menu separator.
If 'name' is nil, the entry defines a "hidden menu item": they are part of the template to HTML
generation process, but they don't appear in the menu. These entries NEED A LINK
AND TITLE FIELDS! See the definition of 'forum.html' below for an example of such item.
--]]

local menu =
{
  { { "eLua Documentation"}, "index.html"},
 -- "Using eLua" -- Menu Separator
    
  { { "Generic info", "Doc Genérica" }, "using.html", 
    {
      { { "Using eLua", "Usando eLua" }, "using.html", 
        {
          { { "Over UART", "Console via UART" }, "using.html#uart" },
          { { "Over TCP/IP", "Console via TCP/IP" }, "using.html#tcpip" },
          { { "On PC", "Num PC" }, "using.html#pc" },
          { { "The shell", "O Shell" }, "using.html#shell" },
          { "Linenoise", "linenoise.html" },
          { "Cross-compiling", "using.html#cross" },
          { "LuaRPC", "using.html#rpc" },
          { "The serial multiplexer", "sermux.html" }
        },
      },
      { "eLua file systems", "filesystems.html",
        {
          { { "Read-Only FS in MCU Flash", "O ROM File System em Flash" }, "arch_romfs.html" },
          { "R/W FAT FS in SD/MMC Cards", "fatfs.html" },
          { "Remote file system (RFS)", "arch_rfs.html" }
        }
      },
      { "eLua interrupt handlers", "inthandlers.html",
        {
          { "In Lua", "inthandlers.html" },
          { "In C", "inthandlers.html#cints" },
        }
      },
      { { "Building eLua", "Build de eLua" }, "building.html",
        {
          { "Building eLua in Linux", "building_unix.html" },
          { "Building eLua in Windows", "building_win.html" },
        },  
      },
    },
  }, 
          
  
  -- "eLua internals" (menu separator)
  { { "eLua internals" }, "",
    { 
      { -- "Overview"      
        { "Overview", "Visão Geral" }, "arch_overview.html", 
        {
          { { "eLua's Architecture", "Arquitetura de eLua" }, "arch_overview.html#structure" },
          { { "Common code", "Código Básico" }, "arch_overview.html#common" },
          { { "Interface architecture", "Interfaceamento" }, "arch_overview.html#platform" },
          { { "Booting eLua", "O Boot de eLua" }, "arch_overview.html#boot" },
          { { "Platforms and ports", "Portabilização" }, "arch_overview.html#platforms" },
          { { "Adding a new port", "Portando eLua" }, "arch_newport.html" },
          { "Implementing interrupts", "arch_ints.html",
            {
              { "Interrupt list", "arch_ints.html#intlist" }
            }
          }
        }
      },
       -- Platform interface (automatically generated)
      { { "Platform interface", "Interfaceamento" }, "arch_platform.html", "arch_platform" },

      -- Other WRITE THE arch_platform_other.html file
      { { "More information", "Informações Adicionais" }, "#", 
        {
          { { "Consoles and terminals", "Consoles e Terminais" }, "arch_con_term.html" },
          { { "TCP/IP in eLua", "TCP/IP em eLua" }, "arch_tcpip.html" },
          { { "LTR (Lua Tiny RAM) in eLua", "LTR (Lua Tiny RAM) em eLua" }, "arch_ltr.html" },
          { "EGC (Emergency GC) in eLua", "elua_egc.html" },
          { { "eLua toolchains", "Toolchains para eLua" }, "toolchains.html" },
          { { "Building toolchains", "Build de Toolchains" }, "tchainbuild.html",
            {
              { "ARM7 and ARM9", "tc_arm.html" },
              { "ARM Cortex-M3", "tc_cortex.html" },
              { "i386", "tc_386.html" }
            },
          },
          { { "eLua coding style", "Regras de Codificação" }, "arch_coding.html" },
        }
      },
    }
  },
  
  {  -- "Reference manual" (menu separator)
    { "Reference Manual", "Manual de Referência" },"",
    {
      -- "Generic modules" (automatically generated)
      { { "Generic modules", "Módulos Genéricos" }, "refman_gen.html", "refman_gen" },

      -- "Platform modules" (automatically generated)
      { { "Platform modules", "Módulos Específicos" }, "refman_dep.html", 
        {
          { "lm3s", "modules_lm3s.html", "refman_ps_lm3s" },
          { "str9", "modules_str9.html", "refman_ps_str9" },
          { "mbed", "modules_mbed.html", "refman_ps_mbed" }
        }
      }
    }
  }
  
}
-- Translations for different strings needed by the generator
-- The order of languages is the same as the one defines in the languages array 
-- defined at the beginning of buildall.lua
local translations = 
{
  { "Overview", "Apresentação" },
  { "Data structures", "Estrutura" },
  { "Functions", "Funções" },
  { "eLua - Embedded Lua", "eLua - Embedded Lua" },
  { "Search", "Pesquisar" },
  { "Language", "Idioma" }
}

-- The "fixed part" are the files/directory that must be present in our final
-- HTML documentation distribution. Directories end with a "/", files don't.
local fixed = 
{
  "images/",
  "js/",
  "css/",
}

-- Return our whole data defined above
return menu, translations, fixed

