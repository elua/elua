-------------------------------------------------------------------------------
-- eLua Documentation Structure Definition
--
-- This is the configuration file for the WebBook html help generator .
-- More info about WebBook can be found @ http://www.tecgraf.puc-rio.br/webbook
--
-------------------------------------------------------------------------------

wb_usr = {
   langs = {"en","pt" },
   start_size = "200",
   contact = "dadosutter@eluaproject.net",
   copyright_link = "http://www.eluaproject.net", 
   search_link = "http://www.eluaproject.net/doc", 
   copyright_name = "eLua Team",
   title_bgcolor = "midnightblue",
   file_title = "wb",
   start_open = "1"
} 

wb_usr.messages = {
  pt = {
     title = "eLua Doc",
     bar_title = "eLua Doc"
  },
  en = {
     title = "eLua Doc",
     bar_title = "eLua Doc"
  }
} 

wb_usr.tree =
{     
  name = { nl= "eLua API" },
  link = "eluaapi.html", 
  folder =
 {
    {
      name = { en = "Generic Modules", pt = "Módulos Genéricos" },
      link = "genericmodules.html",
      folder = 
      {
        {
          name = { en ="bit", pt = "bit" },
          link = "bit_ref.html",
          folder =
          {
	    {
	      name ={ en ="bnot", pt ="bnot" },
	      link ="bit_ref.html#bnot"
	    },
	    {
	      name ={ en ="band", pt ="band" },
	      link ="bit_ref.html#band"
	    },
	    {
	      name ={ en ="bor", pt ="bor" },
	      link ="bit_ref.html#bor"
	    },
	    {
	      name ={ en ="bxor", pt ="bxor" },
	      link ="bit_ref.html#bxor"
	    },
	    {
	      name ={ en ="lshift", pt ="lshift" },
	      link ="bit_ref.html#lshift"
	    },
	    {
	      name ={ en ="rshift", pt ="rshift" },
	      link ="bit_ref.html#rshift"
	    },
	    {
	      name ={ en ="bit", pt ="bit" },
	      link ="bit_ref.html#bit"
	    },
	    {
	      name ={ en ="set", pt ="set" },
	      link ="bit_ref.html#set"
	    },
	    {
	      name ={ en ="clear", pt ="clear" },
	      link ="bit_ref.html#clear"
	    },
	    {
	      name ={ en ="isset", pt ="isset" },
	      link ="bit_ref.html#isset"
	    },
	    {
	      name ={ en ="isclear", pt ="isclear" },
	      link ="bit_ref.html#isclear"
	    },
          },
        },  
        {
          name = { en = "cpu", pt = "cpu" },
          link = "cpu_ref.html",
          folder =
          {
            {
              name ={ en ="write32", pt ="write32" },
              link ="cpu_ref.html#write32"
            },
            {
              name ={ en ="write16", pt ="write16" },
              link ="cpu_ref.html#write16"
            },
            {
              name ={ en ="write8", pt ="write8" },
              link ="cpu_ref.html#write8"
            },
            {
              name ={ en ="reat32", pt ="read32" },
              link ="cpu_ref.html#read32"
            },
            {
              name ={ en ="read16", pt ="read16" },
              link ="cpu_ref.html#read16"
            },
            {
              name ={ en ="read8", pt ="read8" },
              link ="cpu_ref.html#read8"
            },
            {
              name ={ en ="disableinterrupts", pt ="disableinterrupts" },
              link ="cpu_ref.html#disableinterrupts"
            },
            {
              name ={ en ="enableinterrupts", pt ="enableinterrupts" },
              link ="cpu_ref.html#enableinterrupts"
            },
            {
              name ={ en ="clockfrequency", pt ="clockfrequency" },
              link ="cpu_ref.html#clockfrequency"
            },
          },
        },
        {
          name = { en ="gpio", pt = "gpio" },
          link = "gpio_ref.html",
          folder =
          {
	    {
	      name ={ en ="configpin", pt ="configpin" },
	      link ="gpio_ref.html#configpin"
	    },
	    {
	      name ={ en ="setpinvalue", pt ="setpinvalue" },
	      link ="gpio_ref.html#setpinvalue"
	    },
	    {
	      name ={ en ="getpinvalue", pt ="getpinvalue" },
	      link ="gpio_ref.html#getpinvalue"
	    },
	    {
	      name ={ en ="setpinhigh", pt ="setpinhigh" },
	      link ="gpio_ref.html#setpinhigh"
	    },
	    {
	      name ={ en ="setpinlow", pt ="setpinlow" },
	      link ="gpio_ref.html#setpinlow"
	    },
          },
        },  
        {
          name = {en = "net", pt = "net" },
          link = "net_ref.html",
          folder =
          {
            {
              name ={ en ="to be added ...", pt ="setup" },
              link ="net_ref.html#setup"
            },
          },
        },  
        {
          name = { en = "pwm", pt = "pwm" },
          link = "pwm_ref.html",
          folder =
          {
            {
              name ={ en ="setup", pt ="setup" },
              link ="pwm_ref.html#setup"
            },
            {
              name ={ en ="setcycle", pt ="setcycle" },
              link ="pwm_ref.html#setcycle"
            },
            {
              name ={ en ="start", pt ="start" },
              link ="pwm_ref.html#start"
            },
            {
              name ={ en ="stop", pt ="stop" },
              link ="pwm_ref.html#stop"
            },
            {
              name ={ en ="setclock", pt ="setclock" },
              link ="pwm_ref.html#setclock"
            },
            {
              name ={ en ="getclock", pt ="getclock" },
              link ="pwm_ref.html#getclock[A"
            },
          },
        },
        {
          name = { en = "spi", pt = "spi" },
          link = "spi_ref.html",
          folder =
          {
            {
              name ={ en ="setup", pt ="setup" },
              link ="spi_ref.html#setup"
            },
            {
              name ={ en ="select", pt ="select" },
              link ="spi_ref.html#select"
            },
            {
              name ={ en ="unselect", pt ="unselect" },
              link ="spi_ref.html#unselect"
            },
            {
              name ={ en ="send", pt ="send" },
              link ="spi_ref.html#send"
            },
            {
              name ={ en ="sendrecv", pt ="sendrecv" },
              link ="spi_ref.html#sendrecv"
            },
          },
        },
        {
          name = { en = "sys", pt = "sys" },
          link = "sys_ref.html",
          folder =
          {
            {
              name ={ en ="platform", pt ="platform" },
              link ="sys_ref.html#platforms"
            },
            {
              name ={ en ="mcu", pt ="mcu" },
              link ="sys_ref.html#mcu"
            },
            {
              name ={ en ="cpu", pt ="cpu" },
              link ="sys_ref.html#cpu"
            },
            {
              name ={ en ="board", pt ="board" },
              link ="sys_ref.html#board"
            },
          },
        },
        {
          name = {en = "term", pt = "term" },
          link = "term_ref.html",
          folder =
          {
            {
              name ={ en ="clear", pt ="clear" },
              link ="term_ref.html#clear"
            },
            {
              name ={ en ="cleareol", pt ="cleareol" },
              link ="term_ref.html#cleareol"
            },
            {
              name ={ en ="moveto", pt ="moveto" },
              link ="term_ref.html#moveto"
            },
            {
              name ={ en ="moveup", pt ="moveup" },
              link ="term_ref.html#moveup"
            },
            {
              name ={ en ="movedown", pt ="movedown" },
              link ="term_ref.html#movedown"
            },
            {
              name ={ en ="moveleft", pt ="moveleft" },
              link ="term_ref.html#moveleft"
            },
            {
              name ={ en ="moveright", pt ="moveright" },
              link ="term_ref.html#moveright"
            },
            {
              name ={ en ="getlinecount", pt ="getlinecount" },
              link ="term_ref.html#getlinecount"
            },
            {
              name ={ en ="getcolcount", pt ="getcolcount" },
              link ="term_ref.html#getcolcount"
            },
            {
              name ={ en ="printstr", pt ="printstr" },
              link ="term_ref.html#printstr"
            },
            {
              name ={ en ="getx", pt ="getx" },
              link ="term_ref.html#getx"
            },
            {
              name ={ en ="gety", pt ="gety" },
              link ="term_ref.html#gety"
            },
            {
              name ={ en ="inputchar", pt ="inputchar" },
              link ="term_ref.html#inputchar"
            },
          },
        },  
        {
          name = { en = "tmr", pt = "tmr" },
          link = "tmr_ref.html",
          folder =
          {
            {
              name ={ en ="delay", pt ="delay" },
              link ="tmr_ref.html#delay"
            },
            {
              name ={ en ="read", pt ="read" },
              link ="tmr_ref.html#read"
            },
            {
              name ={ en ="start", pt ="start" },
              link ="tmr_ref.html#start"
            },
            {
              name ={ en ="diff", pt ="diff" },
              link ="tmr_ref.html#diff"
            },
            {
              name ={ en ="mindelay", pt ="mindelay" },
              link ="tmr_ref.html#mindelay"
            },
            {
              name ={ en ="maxdelay", pt ="maxdelay" },
              link ="tmr_ref.html#maxdelay"
            },
            {
              name ={ en ="setclock", pt ="setclock" },
              link ="tmr_ref.html#setclock"
            },
          },
        },
        {
          name = {en = "uart", pt = "uart" },
          link = "uart_ref.html",
          folder =
          {
            {
              name ={ en ="setup", pt ="setup" },
              link ="uart_ref.html#setup"
            },
            {
              name ={ en ="send", pt ="send" },
              link ="uart_ref.html#send"
            },
            {
              name ={ en ="recv", pt ="recv" },
              link ="uart_ref.html#recv"
            },
          },
        },  
      },
    },
    {
      name = {en = "Platform Dependent Modules", pt = "Plataformas" },
      link = "platdependentmodules.html",
      folder =
      {
        {  name = { en ="disp", pt = "disp" },
           link = "disp_ref.html",
           folder =
           {
             {
               name ={ en ="init", pt ="init" },
               link ="disp_ref.html#init"
	     },
             {
               name = { en = "enable", pt = "enable" },
               link = "disp_ref.html#enable"
             },
             {
               name = { en = "disable", pt = "disable" },
               link = "disp_ref.html#disable"
             },
             {
               name = { en = "on", pt = "on" },
               link = "disp_ref.html#on"
             },
             {
               name = { en = "off", pt = "off" },
               link = "disp_ref.html#off"
             },
             {
               name = { en = "print", pt = "print" },
               link = "disp_ref.html#print"
             },
             {
               name = { en = "draw", pt = "draw" },
               link = "disp_ref.html#draw"
             },
          },   
        },
      },
    },
  },
} 

