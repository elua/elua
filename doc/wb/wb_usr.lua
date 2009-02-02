-------------------------------------------------------------------------------
-- eLua Documentation Structure Definition
--
-- This is a config file for the WebBook help html generator 
-- More info about WebBook can be found @ http://www.tecgraf.puc-rio.br/webbook
--
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
     bar_title = "eLua"
  },
  en = {
     title = "eLua Doc",
     bar_title = "eLua"
  }
} 

wb_usr.tree =
{     
  name = { nl= "eLua API" },
  link = "api_doc.html", 
  folder =
  {
    {
      name = { en = "Generic Modules", pt = "Módulos Genéricos" },
      link = "genericmodules.html",
      folder = 
      {
        {
          name = { en ="gpio", pt = "gpio" },
          link = "gpio_ref.html",
          folder =
          {
	          {
	            name ={ en ="configpin()", pt ="configpin()" },
	            link ="gpio_ref.html#configpin"
	          },
	          {
	            name ={ en ="setpinvalue()", pt ="setpinvalue()" },
	            link ="gpio_ref.html#setpinvalue"
	          },
	          {
	            name ={ en ="getpinvalue()", pt ="getpinvalue()" },
	            link ="gpio_ref.html#getpinvalue"
	          },
	          {
	            name ={ en ="setpinhigh()", pt ="setpinhigh()" },
	            link ="gpio_ref.html#setpinhigh"
	          },
	          {
	            name ={ en ="setpinlow()", pt ="setpinlow()" },
	            link ="gpio_ref.html#setpinlow"
	          }
          },
        },  
        {
          name = {en = "uart", pt = "uart" },
          link = "uart_ref.html",
          folder =
          {
	          {
	            name ={ en ="setup()", pt ="setup()" },
	            link ="uart_ref.html#setup"
	          },
	          {
	            name ={ en ="send()", pt ="send()" },
	            link ="uart_ref.html#send"
	          },
	          {
	            name ={ en ="recv()", pt ="recv()" },
	            link ="uart_ref.html#recv"
	          }
          },
        },  
        {
          name = { en = "spi", pt = "spi" },
          link = "spi_ref.html",
          folder =
          {
	          {
	            name ={ en ="setup()", pt ="setup()" },
	            link ="spi_ref.html#setup"
	          },
	          {
	            name ={ en ="select()", pt ="select()" },
	            link ="spi_ref.html#select"
	          },
	          {
	            name ={ en ="unselect()", pt ="unselect()" },
	            link ="spi_ref.html#unselect"
	          },
	          {
	            name ={ en ="send()", pt ="send()" },
	            link ="spi_ref.html#send"
	          },
	          {
	            name ={ en ="sendrecv()", pt ="sendrecv()" },
	            link ="spi_ref.html#sendrecv"
	          }
          },
        },
      }
    },
    {
      name = {en = "Platform Dependent", pt = "Plataformas" },
      link = "platdependentmodules.html",
      folder =
      {
        name = { en ="disp", pt = "disp" },
        link = "disp_ref.html",
        folder =
        {
          {
	    name ={ en ="init()", pt ="init()" },
	    link ="disp_ref.html#init"
	  },
	  {
	    name = { en = "enable()", pt = "enable()" },
	    link = "disp_ref.html#enable"
	  },
	  {
	    name = { en = "disable()", pt = "disable()" },
	    link = "disp_ref.html#disable"
	  }
        },
      },
    },
  }
} 

