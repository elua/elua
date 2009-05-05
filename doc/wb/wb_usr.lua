-------------------------------------------------------------------------------
-- eLua Documentation Structure Definition
--
-- This is the configuration file for the WebBook html help generator.
-- It is used to generate the menu tree and it's links to the content pages.
-- More info about WebBook can be found @ http://www.tecgraf.puc-rio.br/webbook
--
-------------------------------------------------------------------------------


-- General config parameters
wb_usr = {
   langs = {"en","pt" },
   start_size = "200",                            -- Menu tree initial width
   title_bar_height = 80,                         -- Title bar initial height
   title_bgcolor = "midnightblue",
   enable_search = true,
   search_link = "http://www.eluaproject.net", 
--   search_label = "Search: ",
--   contact = "eluateam@eluaproject.net",
--   copyright_name = "eLua Team",
--   copyright_link = "http://www.eluaproject.net", 
   file_title = "wb",
   start_open = "1",
   logo_image_file = "eLuaLogo.png",
   logo_onclick_link = ""
} 



-- Page and Bar Titles
wb_usr.messages = 
{
  en = 
  {
    title = "eLua Doc",
    bar_title = "eLua - Embedded Lua"
  },
  pt = 
  {
    title = "eLua Doc",
    bar_title = "eLua - Embedded Lua"
  }
} 



-- Menu Tree Definition
wb_usr.tree =
{
  name = { nl = "eLua" },
  link = "news.html", 
  footer = [[
    <p style="margin-left: 5px;"><a href="http://www.pax.com/free-counters.html"><img src="http://counter.pax.com/counter/image?counter=ctr-zsg80nnmqt" alt="Free Hit Counter" border="0" /></a></p>
  ]],
  folder =
  {
    { name = {en = "Overview", pt = "Apresentação" },
      link = "overview.html",
      folder =
      {
        {  name = { en = "What is", pt = "O que é eLua ?" },
           link = "overview.html#whatis",
        },
        {
          name = { en = "Features", pt = "##Features" },
          link = "overview.html#features",
        },
        {  name = { en = "Audience", pt = "Público alvo" },
           link = "overview.html#audience",
        },
--[[
        {  name = { en = "Uses", pt = "Aplicações" },
           link = "overview.html#uses",
        },
--]]        
        {  name = { en = "Authors", pt = "Autores" },
           link = "overview.html#authors",
        },
        {  name = { en = "Contacts", pt = "Contatos" },
           link = "overview.html#contacts",
        },
        {  name = { en = "License", pt = "Licença" },
           link = "overview.html#license",
        },
      },
    },

    {
      name = { en = "Downloads", pt = "Downloads" },
      link = "downloads.html",
      folder =
      {
        {
          name = { en = "Binaries", pt = "Binaries" },
          link = "downloads.html#binaries",
        },     
        { 
          name = { en = "Source Code", pt = "Fontes" },
          link = "downloads.html#source",
          folder = 
          {
            { 
              name = { en = "Official Releases", pt = "Versões Oficiais[B" },
              link = "downloads.html#official"
            },
            { 
              name = { en = "SVN Public", pt = "SVN Público" },
              link = "downloads.html#svnpublic"
            }, 
            { 
              name = { en = "Developers", pt = "Desenvolvedores" },
              link = "downloads.html#svndev"
            },
          },
        },
        { 
          name = { en = "Old Versions", pt = "Versões Anteriores" },
          link = "dl_old.html",
          folder = 
          {
            { name = { en = "0.4.1", pt = "0.4.1" },
              link = "dl_old.html#v041"
            },
            { name = { en = "0.4", pt = "0.4" },
              link = "dl_old.html#v04"
            },
            { name = { en = "0.3", pt = "0.3" },
              link = "dl_old.html#v03"
            },
            { name = { en = "0.2", pt = "0.2" },
              link = "dl_old.html#v02"
            },
            { name = { en = "0.1", pt = "0.1" },
              link = "dl_old.html#v01"
            },
          },
        },
      },
    },

    {
      name = {en = "Community", pt = "Comunidade" },
      link = "comunity.html",
      folder =
      {
        {  name = { en = "Lists", pt = "Listas" },
           link = "comunity.html#lists",
        },
        { name = { en = "Forums", pt = "##Forums" },
           link = "comunity.html#forums",
        },
        {  name = { en = "Credits", pt = "Créditos" },
           link = "comunity.html#credits",
        },
--[[
        {  name = { en = "Galery", pt = "Galeria" },
           link = "comunity.html#galery",
        },
        {  name = { en = "Projects", pt = "Projetos" },
           link = "comunity.html#projects",
        },
--]]        
      },
    },
    {
      name = { en = "News", pt = "Notícias" },
      link = "news.html",
    },
    {
      name = {en = "Status", pt = "Status" },
      link = "status.html",
      folder =
      {
        {  name = { en = "Platforms Supported", pt = "Plataformas Suportadas" },
           link = "status.html#platforms",
        },
        {  name = { en = "Roadmap", pt = "Futuro" },
           link = "status.html#roadmap",
        },
      },
    },

    {
      name = {en = "Version History", pt = "Histórico de Versões" },
      link = "versionhistory.html",
    },

    {
      name = { en = "", pt = "" },       -- Horizontal Separator
      link = ""
    },
    {
      name = { en = "Documentation", pt = "Documentação" },
      link = "doc.html",
      folder = 
      {
        {
          name = { en = "Building", pt = "Building" },
          link = "building.html",
          folder = 
          {
            { name = { en = "Toolchains", pt = "##Tools" },
              link = "toolchains.html",
            },
          },
        },
        {
          name = { en = "Installing", pt = "##Installing" },
          link = "installing.html",
          folder = 
          {
            {  name = { en = "AT91SAM7X", pt = "AT91SAM7X" },
               link = "installing_at91sam7x.html",
            },
            {   name = { en = "LM3S", pt = "LM3S" },
               link = "installing_lm3s.html",
            },
            {  name = { en = "AVR32", pt = "AVR32" },
               link = "installing_avr32.html",
            },
            {  name = { en = "LPC288x", pt = "LPC288x" },
               link = "installing_lpc2888.html",
            },
            {  name = { en = "STR7", pt = "STR7" },
               link = "installing_str7.html",
            },
            {  name = { en = "STR9", pt = "STR9" },
               link = "installing_str9.html",
            },
            {  name = { en = "STM32", pt = "STM32" },
               link = "installing_stm32.html",
            },
            { name = { en = "i386", pt = "i386" },
               link = "installing_i386.html"
            },
          },
        },
        {
          name = { en = "Using", pt = "Usando" },
          link = "using.html",
          folder = 
          {
            { name = { en = "Over UART", pt = "##Over UART" },
              link = "using.html#uart",
            },
            { name = { en = "Over TCP/IP", pt = "##Over TCP/IP" },
              link = "using.html#tcpip",
            },
            { name = { en = "On PC", pt = "##On PC" },
              link = "using.html#pc",
            },
            { name = { en = "The shell", pt = "##The shell" },
              link = "using.html#shell",
            },
            { name = { en = "Cross-compiling", pt = "##Cross-compiling" },
              link = "using.html#cross",
            },
          },
        },
        {
          name = { en = "FAQ", pt = "FAQ" },
          link = "faq.html",
        },
        { name = { en = "Architecture", pt = "##Architecture" },
          link = "arch.html",
          folder = 
          {
            { name = { en = "Overview", pt = "##Overview" },
              link = "arch_overview.html",
              folder = 
              {
                { name = { en = "Structure", pt = "##Structure" },
                  link = "arch_overview.html#structure",
                },
                { name = { en = "Common code", pt = "##Common code" },
                  link = "arch_overview.html#common",
                },
                { name = { en = "Platform interface", pt = "##Platform interface" },
                  link = "arch_overview.html#platform",
                },
                { name = { en = "Platforms/ports", pt = "##Platforms/ports" },
                  link = "arch_overview.html#platforms",
                },
                { name = { en = "Booting eLua", pt = "##eLua boot process" },
                  link = "arch_overview.html#boot",
                },
              },
            },
            { name = { en = "Platform interface", pt = "##Platform interface" },
              link = "arch_platform.html",
              folder = 
              {
                { name = { en = "PIO", pt = "PIO" },
                  link = "arch_platform_pio.html",
                }, 
                { name = { en = "SPI", pt = "SPI" },
                  link = "arch_platform_spi.html",
                }, 
                { name = { en = "UART", pt = "UART" },
                  link = "arch_platform_uart.html",
                }, 
                { name = { en = "Timers", pt = "##Timers" },
                  link = "arch_platform_timers.html",
                }, 
                { name = { en = "PWM", pt = "PWM" },
                  link = "arch_platform_pwm.html",
                }, 
                { name = { en = "CPU", pt = "CPU" },
                  link = "arch_platform_cpu.html",
                }, 
                { name = { en = "ADC", pt = "ADC" },
                  link = "arch_platform_adc.html",
                }, 
                { name = { en = "Ethernet", pt = "ethernet" },
                  link = "arch_platform_eth.html",
                }, 
              },
            },
            { name = { en = "ROM file system", pt = "##ROM file system" },
              link = "arch_romfs.html",
            },
            { name = { en = "Adding a new port", pt = "##Adding a new port" },
              link = "arch_newport.html",
            },
            { name = { en = "Modules and LTR", pt = "##Module and LTR" },
              link = "arch_ltr.html",
            },
            { name = { en = "Consoles and terminals", pt = "##Consoles and terminals" },
              link = "arch_con_term.html",
            },
            { name = { en = "TCP/IP in eLua", pt = "##TCP/IP in eLua" },
              link = "arch_tcpip.html",
            },
            { name = { en = "eLua coding style", pt = "##eLua coding style" },
              link = "arch_coding.html",
            },
          }
        },
        {
          name = { en = "Examples", pt = "Exemplos" },
          link = "examples.html",
        },
      },
    },

    {
      name = { en = "Tutorials", pt = "Tutoriais" },
      link = "tutorials.html",
      folder =
      {
        {  name = { en = "Booting on a PC", pt = "Booting on a PC" },
           link = "tut_bootpc.html",
        },
        {  name = { en = "Booting from a Pendrive", pt = "Booting from a Pendriv" },
           link = "tut_bootstick.html",
        },
        {  name = { en = "Toolchain Building", pt = "Toolchain Building" },
           link = "tchainbuild.html",
           folder = 
           {
             { name = { en = "ARM7 & ARM9 Toolchains", pt = "ARM 7 & ARM9 Toolchains" },
               link = "tc_arm.html",
             },
             { name = { en = "ARM Cortex-M3", pt = "ARM Cortex-M3" },
               link = "tc_cortex.html",
             },
             { name = { en = "i386", pt = "i386" },
               link = "tc_386.html",
             },
           },  
        },
        {  name = { en = "Using OpenOCD", pt = "Using OpenOC" },
           link = "tut_openocd.html",
        },
      },
    },


    {
      name = { en = "Reference Manual", pt = "Manual de Referência" },
      link = "refman.html#genericmodules",
      folder = 
      {
        {
          name = { en = "Generic Modules", pt = "Módulos Genéricos" },
          link = "refman.html#genericmodules",
          folder = 
          {
            {
              name = { en = "bit", pt = "bit" },
              link = "refman.html#bitmodule",
              folder =
              {
                {
                  name ={ en = "bnot", pt = "bnot" },
                  link = "refman.html#bit_bnot"
                },
                {
                  name ={ en = "band", pt = "band" },
                  link = "refman.html#bit_band"
                },
                {
                  name ={ en = "bor", pt = "bor" },
                  link = "refman.html#bit_bor"
                },
                {
                  name ={ en = "bxor", pt = "bxor" },
                  link = "refman.html#bit_bxor"
                },
                {
                  name ={ en = "lshift", pt = "lshift" },
                  link = "refman.html#bit_lshift"
                },
                {
                  name ={ en = "rshift", pt = "rshift" },
                  link = "refman.html#bit_rshift"
                },
                {
                  name ={ en = "bit", pt = "bit" },
                  link = "refman.html#bit_bit"
                },
                {
                  name ={ en = "set", pt = "set" },
                  link = "refman.html#bit_set"
                },
                {
                  name ={ en = "clear", pt = "clear" },
                  link = "refman.html#bit_clear"
                },
                {
                  name ={ en = "isset", pt = "isset" },
                  link = "refman.html#bit_isset"
                },
                {
                  name ={ en = "isclear", pt = "isclear" },
                  link = "refman.html#bit_isclear"
                },
              },
            },  
            {
              name = { en = "cpu", pt = "cpu" },
              link = "refman.html#cpumodule",
              folder =
              {
                {
                  name ={ en = "write32", pt = "write32" },
                  link = "refman.html#cpu_write32"
                },
                {
                  name ={ en = "write16", pt = "write16" },
                  link = "refman.html#cpu_write16"
                },
                {
                  name ={ en = "write8", pt = "write8" },
                  link = "refman.html#cpu_write8"
                },
                {
                  name ={ en = "reat32", pt = "read32" },
                  link = "refman.html#cpu_read32"
                },
                {
                  name ={ en = "read16", pt = "read16" },
                  link = "refman.html#cpu_read16"
                },
                {
                  name ={ en = "read8", pt = "read8" },
                  link = "refman.html#cpu_read8"
                },
                {
                  name ={ en = "disableinterrupts", pt = "disableinterrupts" },
                  link = "refman.html#cpu_disableinterrupts"
                },
                {
                  name ={ en = "enableinterrupts", pt = "enableinterrupts" },
                  link = "refman.html#cpu_enableinterrupts"
                },
                {
                  name ={ en = "clockfrequency", pt = "clockfrequency" },
                  link = "refman.html#cpu_clockfrequency"
                },
              },
            },
            {
              name = { en = "gpio", pt = "gpio" },
              link = "refman.html#gpiomodule",
              folder =
              {
                {
                  name ={ en = "configpin", pt = "configpin" },
                  link = "refman.html#gpio_configpin"
                },
                {
                  name ={ en = "setpinvalue", pt = "setpinvalue" },
                  link = "refman.html#gpio_setpinvalue"
                },
                {
                  name ={ en = "getpinvalue", pt = "getpinvalue" },
                  link = "refman.html#gpio_getpinvalue"
                },
                {
                  name ={ en = "setpinhigh", pt = "setpinhigh" },
                  link = "refman.html#gpio_setpinhigh"
                },
                {
                  name ={ en = "setpinlow", pt = "setpinlow" },
                  link = "refman.html#gpio_setpinlow"
                },
              },
            },  
            {
              name = {en = "net", pt = "net" },
              link = "refman.html#netmodule",
              folder =
              {
                {
                  name ={ en = "to be added ...", pt = "setup" },
                  link = "net_ref.html#net_setup"
                },
              },
            },  
            {
              name = { en = "pwm", pt = "pwm" },
              link = "refman.html#pwmmodule",
              folder =
              {
                {
                  name ={ en = "setup", pt = "setup" },
                  link = "refman.html#pwm_setup"
                },
                {
                  name ={ en = "setcycle", pt = "setcycle" },
                  link = "refman.html#pwm_setcycle"
                },
                {
                  name ={ en = "start", pt = "start" },
                  link = "refman.html#pwm_start"
                },
                {
                  name ={ en = "stop", pt = "stop" },
                  link = "refman.html#pwm_stop"
                },
                {
                  name ={ en = "setclock", pt = "setclock" },
                  link = "refman.html#pwm_setclock"
                },
                {
                  name ={ en = "getclock", pt = "getclock" },
                  link = "refman.html#pwm_getclock"
                },
              },
            },
            {
              name = { en = "spi", pt = "spi" },
              link = "refman.html#spimodules",
              folder =
              {
                {
                  name ={ en = "setup", pt = "setup" },
                  link = "refman.html#spi_setup"
                },
                {
                  name ={ en = "select", pt = "select" },
                  link = "refman.html#spi_select"
                },
                {
                  name ={ en = "unselect", pt = "unselect" },
                  link = "refman.html#spi_unselect"
                },
                {
                  name ={ en = "send", pt = "send" },
                  link = "refman.html#spi_send"
                },
                {
                  name ={ en = "sendrecv", pt = "sendrecv" },
                  link = "refman.html#spi_sendrecv"
                },
              },
            },
            {
              name = { en = "sys", pt = "sys" },
              link = "refman.html#sysmodule",
              folder =
              {
                {
                  name ={ en = "platform", pt = "platform" },
                  link = "refman.html#sys_platforms"
                },
                {
                  name ={ en = "mcu", pt = "mcu" },
                  link = "refman.html#sys_mcu"
                },
                {
                  name ={ en = "cpu", pt = "cpu" },
                  link = "refman.html#sys_cpu"
                },
                {
                  name ={ en = "board", pt = "board" },
                  link = "refman.html#sys_board"
                },
              },
            },
            {
              name = {en = "term", pt = "term" },
              link = "refman.html#termmodule",
              folder =
              {
                {
                  name ={ en = "clear", pt = "clear" },
                  link = "refman.html#term_clear"
                },
                {
                  name ={ en = "cleareol", pt = "cleareol" },
                  link = "refman.html#term_cleareol"
                },
                {
                  name ={ en = "moveto", pt = "moveto" },
                  link = "refman.html#term_moveto"
                },
                {
                  name ={ en = "moveup", pt = "moveup" },
                  link = "refman.html#term_moveup"
                },
                {
                  name ={ en = "movedown", pt = "movedown" },
                  link = "refman.html#term_movedown"
                },
                {
                  name ={ en = "moveleft", pt = "moveleft" },
                  link = "refman.html#term_moveleft"
                },
                {
                  name ={ en = "moveright", pt = "moveright" },
                  link = "refman.html#term_moveright"
                },
                {
                  name ={ en = "getlinecount", pt = "getlinecount" },
                  link = "refman.html#term_getlinecount"
                },
                {
                  name ={ en = "getcolcount", pt = "getcolcount" },
                  link = "refman.html#term_getcolcount"
                },
                {
                  name ={ en = "printstr", pt = "printstr" },
                  link = "refman.html#term_printstr"
                },
                {
                  name ={ en = "getx", pt = "getx" },
                  link = "refman.html#term_getx"
                },
                {
                  name ={ en = "gety", pt = "gety" },
                  link = "refman.html#term_gety"
                },
                {
                  name ={ en = "inputchar", pt = "inputchar" },
                  link = "refman.html#term_inputchar"
                },
              },
            },  
            {
              name = { en = "tmr", pt = "tmr" },
              link = "refman.html#tmrmodule",
              folder =
              {
                {
                  name ={ en = "delay", pt = "delay" },
                  link = "refman.html#tmr_delay"
                },
                {
                  name ={ en = "read", pt = "read" },
                  link = "refman.html#tmr_read"
                },
                {
                  name ={ en = "start", pt = "start" },
                  link = "refman.html#tmr_start"
                },
                {
                  name ={ en = "diff", pt = "diff" },
                  link = "refman.html#tmr_diff"
                },
                {
                  name ={ en = "mindelay", pt = "mindelay" },
                  link = "refman.html#tmr_mindelay"
                },
                {
                  name ={ en = "maxdelay", pt = "maxdelay" },
                  link = "refman.html#tmr_maxdelay"
                },
                {
                  name ={ en = "setclock", pt = "setclock" },
                  link = "refman.html#tmr_setclock"
                },
              },
            },
            {
              name = {en = "uart", pt = "uart" },
              link = "refman.html#uartmodule",
              folder =
              {
                {
                  name ={ en = "setup", pt = "setup" },
                  link = "refman.html#uart_setup"
                },
                {
                  name ={ en = "send", pt = "send" },
                  link = "refman.html#uart_send"
                },
                {
                  name ={ en = "recv", pt = "recv" },
                  link = "refman.html#uart_recv"
                },
              },
            },  
          },
        },
        {
          name = {en = "Platform Dependent Modules", pt = "Dependentes de Plataforma" },
          link = "refman.html#platdepmodules",
          folder =
          {
            {  name = { en = "adc", pt = "adc" },
               link = "refman.html#adcmodule",
               folder =
               {
                 {
                   name ={ en = "sample", pt = "sample" },
                   link = "refman.html#adc_sample"
                 },
                 {
                   name ={ en = "getsamples", pt = "getsamples" },
                   link = "refman.html#adc_getsamples"
                 },
                 {
                   name = { en = "maxval", pt = "maxval" },
                   link = "refman.html#adc_maxval"
                 },
                 {
                   name = { en = "samplesready", pt = "samplesready" },
                   link = "refman.html#adc_samplesready"
                 },
                 {
                   name = { en = "dataready", pt = "dataready" },
                   link = "refman.html#adc_dataready"
                 },
                 {
                   name = { en = "setmode", pt = "setmode" },
                   link = "refman.html#adc_setmode"
                 },
                 {
                   name = { en = "setsmoothing", pt = "setsmoothing" },
                   link = "refman.html#adc_setsmoothing"
                 },
                 {
                   name = { en = "getsmoothing", pt = "getsmoothing" },
                   link = "refman.html#adc_getsmoothing"
                 },
                 {
                   name = { en = "burst", pt = "burst" },
                   link = "refman.html#adc_burst"
                 },
              },   
            },
            {  name = { en = "disp", pt = "disp" },
               link = "refman.html#dispmodule",
               folder =
               {
                 {
                   name ={ en = "init", pt = "init" },
                   link = "refman.html#disp_init"
                 },
                 {
                   name = { en = "enable", pt = "enable" },
                   link = "refman.html#disp_enable"
                 },
                 {
                   name = { en = "disable", pt = "disable" },
                   link = "refman.html#disp_disable"
                 },
                 {
                   name = { en = "on", pt = "on" },
                   link = "refman.html#disp_on"
                 },
                 {
                   name = { en = "off", pt = "off" },
                   link = "refman.html#disp_off"
                 },
                 {
                   name = { en = "clear", pt = "clear" },
                   link = "refman.html#disp_clear"
                 },
                 {
                   name = { en = "print", pt = "print" },
                   link = "refman.html#disp_print"
                 },
                 {
                   name = { en = "draw", pt = "draw" },
                   link = "refman.html#disp_draw"
                 },
              },   
            },
          },
        },
      },
    },
  },
}
