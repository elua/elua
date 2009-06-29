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
          name = { en = "Features", pt = "Características" },
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
        { name = { en = "Forums", pt = "Fórums" },
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
            { name = { en = "Toolchains", pt = "Toolchains" },
              link = "toolchains.html",
            },
          },
        },
        {
          name = { en = "Installing", pt = "Instalando" },
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
            { name = { en = "Over UART", pt = "UART" },
              link = "using.html#uart",
            },
            { name = { en = "Over TCP/IP", pt = "TCP/IP" },
              link = "using.html#tcpip",
            },
            { name = { en = "On PC", pt = "no PC" },
              link = "using.html#pc",
            },
            { name = { en = "The shell", pt = "O Shell" },
              link = "using.html#shell",
            },
            { name = { en = "Cross-compiling", pt = "Cross Compiling" },
              link = "using.html#cross",
            },
          },
        },
        {
          name = { en = "FAQ", pt = "FAQ" },
          link = "faq.html",
        },
        { name = { en = "Architecture", pt = "Arquitetura" },
          link = "arch.html",
          folder = 
          {
            { name = { en = "Overview", pt = "Visão Geral" },
              link = "arch_overview.html",
              folder = 
              {
                { name = { en = "Structure", pt = "Estrutura" },
                  link = "arch_overview.html#structure",
                },
                { name = { en = "Common code", pt = "Código base" },
                  link = "arch_overview.html#common",
                },
                { name = { en = "Platform interface", pt = "Plataformas" },
                  link = "arch_overview.html#platform",
                },
                { name = { en = "Platforms/ports", pt = "Plataformas" },
                  link = "arch_overview.html#platforms",
                },
                { name = { en = "Booting eLua", pt = "Bootando eLua" },
                  link = "arch_overview.html#boot",
                },
              },
            },
            { name = { en = "Platform interface", pt = "Módulos" },
              link = "arch_platform.html",
              folder = {
                $$ARCH_PLATFORM$$
              }
            },
            { name = { en = "ROM file system", pt = "ROM File System" },
              link = "arch_romfs.html",
            },
            { name = { en = "Adding a new port", pt = "Adicionando um novo Port" },
              link = "arch_newport.html",
            },
            { name = { en = "Modules and LTR", pt = "Módulos e LTR" },
              link = "arch_ltr.html",
            },
            { name = { en = "Consoles and terminals", pt = "Consoles e Terminais"},
              link = "arch_con_term.html",
            },
            { name = { en = "TCP/IP in eLua", pt = "TCP/IP em eLua" },
              link = "arch_tcpip.html",
            },
            { name = { en = "eLua coding style", pt = "Estilo de Código" },
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
      link = "refman.html",
      folder = 
      {
        {
          name = { en = "Generic Modules", pt = "Módulos Genéricos" },
          link = "refman_gen.html",
          folder = {
            $$REFMAN_GEN$$        
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
