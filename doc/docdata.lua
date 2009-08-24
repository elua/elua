-- Documentation structure is defined here

--[[
The menu is defined by an array of { name, link, submenu, title } arrays

'name' can be another array keeping names for all languages, in the same order 
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
  -- "Overview"
  { "Overview", "overview.html", 
    {
      { "About", "overview.html#whatis" },
      { "Features", "overview.html#features" },
      { "Audience", "overview.html#audience" },
      { "Authors", "overview.html#authors" },
      { "Contacts", "overview.html#contacts" },
      { "License", "overview.html#license" }
    }
  },

  -- "Downloads"
  { "Downloads", "downloads.html", 
    {
      { "Binaries", "downloads.html#binaries" },
      { "Source code", "downloads.html#source", 
        {
          { "Official releases", "downloads.html#official" },
          { "Bleeding edge (SVN)", "downloads.html#svnpublic" },
          { "Developers", "downloads.html#svndev" },
        }
      },
      { "Old versions", "dl_old.html", 
        {
          { "0.4.1", "dl_old.html#v041" },
          { "0.4", "dl_old.html#v04" },
          { "0.3", "dl_old.html#v03" },
          { "0.2", "dl_old.html#v02" },
          { "0.1", "dl_old.html#v01" }
        }
      },
    }
  },

  -- "Community"
  { "Community", "comunity.html", 
    {
      { "Lists", "comunity.html#lists" },
      { "Forums", "comunity.html#forums" },
      { "Credits", "comunity.html#credits" },
      { nil, "forum.html", nil, "Forum" } -- "hidden" entry (doesn't appear in the menu)
    }
  },

  -- "News"
  { "News", "news.html" },

  -- "Status"
  { "Status", "status.html", 
    {
      { "Supported platforms", "status.html#platforms" },
      { "Roadmap", "status.html#roadmap" }
    }
  },

  -- "Version history"
  { "Version history", "versionhistory.html" },

  -- "Getting started"
  { "Getting started", "doc.html", 
    {
      { "Building", "building.html", 
        {
          { "Toolchains", "toolchains.html" }
        }
      },
      { "Installing", "installing.html", 
        {
          { "AT91SAM7x", "installing_at91sam7x.html" },
          { "LM3S", "installing_lm3s.html" },
          { "AVR32", "installing_avr32.html" },
          { "LPC2888", "installing_lpc2888.html" },
          { "STR7", "installing_str7.html" },
          { "STR9", "installing_str9.html" },
          { "STM32", "installing_stm32.html" },
          { "i386", "installing_i386.html" }
        }
      },
      { "Using", "using.html", 
        {
          { "Over UART", "using.html#uart" },
          { "Over TCP/IP", "using.html#tcpip" },
          { "On PC", "using.html#pc" },
          { "The shell", "using.html#shell" },
          { "Cross-compiling", "using.html#cross" }
        }
      },
      { "Examples", "examples.html" }
    },
  },

  -- "FAQ"
  { "FAQ", "faq.html" },

  -- "eLua internals" (menu separator)
  { "eLua internals" },

  -- "Overview"
  { "Overview", "arch_overview.html", 
    {
      { "Structure", "arch_overview.html#structure" },
      { "Commond code", "arch_overview.html#common" },
      { "Platform interface", "arch_overview.html#platform" },
      { "Platforms/ports", "arch_overview.html#platforms" },
      { "Booting eLua", "arch_overview.html#boot" }
    }
  },

  -- Platform interface (automatically generated)
  { "Platform interface", "arch_platform.html", "arch_platform" },

  -- Other WRITE THE arch_platform_other.html file
  { "Other", "#", 
    {
      { "ROM file system", "arch_romfs.html" },
      { "Adding a new port", "arch_newport.html" },
      { "Modules and LTR", "arch_ltr.html" },
      { "Consoles and terminals", "arch_con_term.html" },
      { "TCP/IP in eLua", "arch_tcpip.html" },
      { "eLua coding style", "arch_coding.html" }
    }
  },

  -- "Reference manual" (menu separator)
  { "Reference manual" },

  -- "Generic modules" (automatically generated)
  { "Generic modules", "refman_gen.html", "refman_gen" },

  -- "Platform modules"
  { "Platform modules", "refman_dep.html", 
    {
      { "lm3s", "modules_lm3s.html", "refman_ps_lm3s" }
    }
  }
}

-- Translations for different strings needed by the generator
-- The order of languages is the same as the one defines in the languages array 
-- defined at the beginning of buildall.lua
local translations = 
{
  { "Overview" },
  { "Data structures" },
  { "Functions" },
  { "eLua - embedded Lua" },
  { "Search" },
  { "Language" }
}

-- The "fixed part" are the files/directory that must be present in our final
-- HTML documentation distribution. Directories end with a "/", files don't.
local fixed = 
{
  "style1.css",
  "images/",
  "ddlevelsfiles/"
}

-- Return our whole data
return menu, translations, fixed

