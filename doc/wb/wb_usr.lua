-------------------------------------------------------------------------------
-- eLua Documentation Structure Definition
--
-- This is a config file for the WebBook help html generator 
-- More info about WebBook can be found @ http://www.tecgraf.puc-rio.br/webbook
--
--
-------------------------------------------------------------------------------

wb_usr = {
   langs = {"en","pt"},
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
     bar_title = "eLua"
  },
  en = {
     title = "eLua Doc",
     bar_title = "eLua"
  }
} 

wb_usr.tree =
{     
  name= {nl= "eLua API"},
  link= "api_doc.html", 
  folder=
  {
    {
      name= {en= "Modules", pt= "Módulos"},
      link= "modules_doc.html",
      folder= 
      {
        {
          name= {en="gpio", pt= "gpio"},
          link= "gpio_doc.html"
        },
        {
          name= {en= "term", pt= "term"},
          link= "term_doc.html"
        },
        {
          name= {en = "bit", pt = "bit"},
          link= "bit_doc.html"
        },
        {
          name= {en = "uart", pt = "uart"},
          link= "uart_doc.html"
        },
        {
          name= {en = "spi", pt = "spi"},
          link= "spi_doc.html"
        },
      }
    },
    {
      name= {en = "Reference", pt = "Referência"},
      link= "ref.html",
      folder=
      {
        name= {en="gpio", pt= "gpio"},
        link= "gpio_ref.html",
        folder=
        {
          name={en="gpio.fun1", pt="gpio.fun1"},
          link="gpio_ref.html#fun1"
        },
        {
          name= {en= "term", pt= "term"},
          link= "gpio_ref.html#fun2"
        }
      },
      {
        name= {en = "bit", pt = "bit"},
        link= "bit_ref.html",
        folder=
        {
          name={en="bit.fun1", pt="bit.fun1"},
          link="bit_ref.html#fun1"
        },
        {
          name={en="bit.fun2", pt="bit.fun2"},
          link="bit_ref.html#fun2"
        },
      },
    },
  }
} 

