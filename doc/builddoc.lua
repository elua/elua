-- eLua doc builder tool 

-- List here all the components of the platform interface
local arch_platform_components = { "ll", "pio", "spi", "uart", "timers", "pwm", "cpu", "eth" }
-- local arch_platform_components = { "cpu" }

-- List here all languages for the documentation (make sure to keep English ("en") the first one)
local languages = { "en", "pt" }
-- Also list here the translation for a bunch of fixed strings
local overview_tr = { en = "Overview", pt = "##Overview" }
local structures_tr = { en = "Data structures", pt = "##Data structures" }
local functions_tr = { en = "Functions", pt = "##Functions" }
local pi_tr = { en = "Platform interface", pt = "##Platform interface" }

-- Format a name to a link by changing all the spaces to "_" and
-- making all letters lowercase
local function name2link( str )
  str = str:gsub( " ", "_" )
  return str:lower()
end

-- Returns the part of the string enclosed between two '#' chars
-- Used for parsing function sig. 
local function namefromsig( str )
  local _, _, name = str:find( "#(.*)#" )
  return name
end

--[[ Process the given string as follows:
- $string$ becomes <b>string</b>
- %string% becomes <i>string</i>
- @ref@text@ becomes <a href="ref">text</a>
- ^ref^text^ becomes <a target="_blank" href="ref">text</a>
- $$, %%, @@, ^^ become $, %, @, ^ respectively
- the string "eLua" becomes <b>eLua</b>
- strings between two tildas (~~) get special code-like formatting
- newlines are changed to ' ' if 'keepnl' isn't true
--]]
local function format_string( str, keepnl )
  -- replace double "special chars" with "temps" for later use
  str = str:gsub( "%$%$", "\001" )
  str = str:gsub( "%%%%", "\002" )
  str = str:gsub( "@@", "\003" )
  str = str:gsub( "%^%^", "\004" )
  str = str:gsub( "~~", "\005" )

  -- replace eLua with <b>eLua</b>
  str = str:gsub( "eLua", "<b>eLua</b>" )

  -- $string$ becomes <b>string></b>
  str = str:gsub( "%$([^%s][^%$]+)%$", "<b>%1</b>" )

  -- %string% becomes <i>string</i>
  str = str:gsub( "%%([^%s][^%%]+)%%", "<i>%1</i>" )

  -- @ref@text@ becomes <a href="ref">text</a>
  str = str:gsub( "@([^%s][^@]+)@([^%s][^@]+)@", '<a href="%1">%2</a>' )

  -- ^ref^text^ becomes <a target="_blank" href="ref">text</a>
  str = str:gsub( "%^([^%s][^%^]+)%^([^%s][^%^]+)%^", '<a target="_blank" href="%1">%2</a>' )

  -- strings between two tildas (~~) get special code-like formatting
  str = str:gsub( "~([^%s][^~]+)~", function( x )
    x = x:gsub( "\n", "<br>" )
    x = x:gsub( "%s%s+", function( x ) return ( "&nbsp;" ):rep( #x ) end )
    return "<p><code>" .. x .. "</code></p>"
  end )
  str = str:gsub( "~~", "~" )

  -- other "\n" chars should dissapear now
  if not keepnl then  str = str:gsub( "\n", " " ) end

  -- put back the "temps"
  str = str:gsub( "\001", "%$" )
  str = str:gsub( "\002", "%%" )
  str = str:gsub( "\003", "@" )
  str = str:gsub( "\004", "%^" )
  str = str:gsub( "\005", "~" )

  -- all done
  return str
end

local header = [[
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Strict//EN">
<html><head>
<meta http-equiv="content-type" content="text/html; charset=ISO-8859-1">
<meta http-equiv="Content-Language" content="en-us"><title>%s</title>

<link rel="stylesheet" type="text/css" href="../style.css"></head>
<body style="background-color: rgb(255, 255, 255);">]]

-- Build the documentation starting from the given file
local function build_file( fname )
  dofile( fname )
  local res = {}

  for _, lang in pairs( languages ) do
    res[ lang ] = {}
    res[ lang ].wb = {}
    local wb = res[ lang ].wb
    
    -- we need english always
    -- the other languages will be substituted with english if not found
    local resname = string.format( "data_%s", lang )
    local r = _G[ resname ]
    if not r then
      if lang == "en" then
        return false, "data_en must exist in the description"
      else
        print( string.format( "'%s': data for language '%s' not found, defaulting to english", fname, lang ) )
        r = _G[ "data_en" ]
      end
    end

    -- process names
    if not r.menu_name then
      return false, "menu_names not found"
    end
    wb.name = r.menu_name

    -- process title
    if not r.title then
      return false, "title not found"
    end
    local page = header:format( r.title ) .. "\n\n"

    -- process overview
    if not r.overview then
      return false, "overview not found"
    end
    page = page .. '<a name="overview"><h3>Overview</h3></a>\n<p>' .. format_string( r.overview ) .. "</p>\n\n"

    -- process structures if needed
    if r.structures then
      local structures = r.structures
      wb.structs = {}
      page = page .. '<a name="structures"><h3>Data structures, constants and types</h3></a>\n'
      for i = 1, #structures do
        local s = structures[ i ]
        wb.structs[ #wb.structs + 1 ] = s.name
        if not s.text or not s.desc or not s.name then
          return false, "structure without text, desc or name fields"
        end
        -- text/name. The link name is ALWAYS the one in ENGLISH.
        page = page .. string.format( '<a name="%s">', name2link( res.en.wb.structs[ i ] ) )
        page = page .. "<p><pre><code>" .. format_string( s.text, true ) .. "</code></pre></p>"
        page = page .. "</a>"
        -- description
        page = page .. "\n<p>" .. format_string( s.desc ) .. "</p>\n\n"
      end 
    end

    -- process functions now
    if not r.funcs then
      return false, "funcs not found"
    end
    local funcs = r.funcs
    page = page .. '<a name="funcs"><h3>Functions</h3></a>\n'
    wb.funcs = {}
    for i = 1, #funcs do
      local f = funcs[ i ]
      if not f.sig or not f.desc then
        return false, "function without sig or desc fields"
      end
      local funcname = namefromsig( f.sig )
      if not funcname then
        return false, string.format( "'%s' should contain the function name between '*' chars", f.sig )
      end
      wb.funcs[ #wb.funcs + 1 ] = funcname
      -- signature
      page = page .. string.format( '<a name="%s">', funcname )
      page = page .. "<p><pre><code>" .. f.sig:gsub( '#', '' ) .. "</code></pre></p>"
      page = page .. "</a>"
      -- description
      page = page .. "\n<p>" .. format_string( f.desc ) .. "</p>\n"
      -- arguments
      page = page .. "<p><b>Arguments</b>: "
      if f.args then
        local a = f.args
        if #a == 1 then
          local t = a[ 1 ]
          page = page .. "<b>" .. t.name .. "</b> - " .. format_string( t.desc )
        else
          page = page .. "\n<ul>\n"
          for i = 1, #a do
            local t = a[ i ]
            page = page .. "  <li><b>" .. t.name .. "</b> - " .. format_string( t.desc) .. "</li>\n"
          end
          page = page .. "</ul>"
        end
      else
        page = page .. "none"
      end
      page = page .. "</p>\n"
      -- return value
      page = page .. "<p><b>Returns</b>: "
      if f.ret then
        local r = f.ret
        if type( r ) == "string" or ( type( r ) == "table" and #r == 1 ) then
          local text = type( r ) == "string" and r or r[ 1 ]
          page = page .. format_string( text )
        else
          page = page .. "\n<ul>\n"
          for i = 1, #r do page = page .. "  <li>" .. format_string( r[ i ] ) .. "</li>\n" end
          page = page .. "</ul>"
        end
      else
        page = page .. "nothing"
      end
      page = page .. "</p>\n\n"
    end

    -- aux data (if any)
    if r.auxdata then
      local auxdata = r.auxdata
      wb.auxdata = {}
      for i = 1, #auxdata do
        local a = auxdata[ i ]
        wb.auxdata[ #wb.auxdata + 1 ] = a.title
        if not a.title or not a.desc then
          return false, "auxdata without title or desc"
        end
        -- the link name is ALWAYS the one in ENGLISH
        page = page .. string.format( '<a name="%s">', name2link( res.en.wb.auxdata[ i ] ) )
        page = page .. "<h3>" .. a.title .. "</h3>"
        page = page .. "</a>"
        page = page .. "\n<p>" .. format_string( a.desc ) .. "</p>\n\n"
      end
    end

    -- footer
    page = page .. "</body></html>\n"
    res[ lang ].page = page
  end
  return res
end

-- Helper function to get strings in all languages when needed
local function alllangs( getstr )
  local str =  "name = {"
  for _, lang in pairs( languages ) do
    str = str .. string.format( ' %s = "%s",', lang, getstr( lang ) )
  end
  return str .. " }"
end

-- Transform the data from the wb dictionary (in 'fulldata') for module 'modname' to a wb string
local function wb2str( fulldata, modname )
  local relfname = "arch_platform_" .. modname .. ".html"
  local res = fulldata[ modname ]
  local wbstr = string.format( '    { %s,\n      link = "%s",\n      folder =\n      {\n', alllangs( function( x ) return res[ x ].wb.name end ), relfname )

  -- Overview
  wbstr = wbstr .. string.format( '        { %s,\n          link = "%s#overview"\n        },\n', alllangs( function( x ) return overview_tr[ x ] end ), relfname )

  -- Data structures (if needed)
  if res.en.wb.structs then
    wbstr = wbstr .. string.format( '        { %s,\n          link = "%s#structures",\n          folder =\n          {\n', alllangs( function( x ) return structures_tr[ x ] end ), relfname )
    for i = 1, #res.en.wb.structs do
      local v = res.en.wb.structs[ i ]
      wbstr = wbstr .. string.format( '            { %s,\n              link = "%s#%s",\n            },\n', alllangs( function( x ) return res[ x ].wb.structs[ i ] end ), relfname, name2link( v ) )
    end
    wbstr = wbstr .. "          }\n        },\n"
  end

  -- Functions
  wbstr = wbstr .. string.format( '        { %s,\n          link = "%s#funcs",\n          folder =\n          {\n', alllangs( function( x ) return functions_tr[ x ] end ), relfname )
  for _, v in pairs( res.en.wb.funcs ) do
    wbstr = wbstr .. string.format( '            { name = { en = "%s", pt = "%s" },\n              link = "%s#%s",\n            },\n', v, v, relfname, name2link( v ) )
  end
  wbstr = wbstr .. "          }\n        },\n"

  -- Aux data (if needed)
  if res.en.wb.auxdata then
    for i = 1, #res.en.wb.auxdata do
      local v = res.en.wb.auxdata[ i ]
      wbstr = wbstr .. string.format(  '        { %s,\n          link = "%s#%s"\n        },\n', alllangs( function( x ) return res[ x ].wb.auxdata[ i ] end ), relfname, name2link( v ) )
    end
  end

  -- Close the structure and return
  wbstr = wbstr .. "      }\n    },\n"
  return wbstr
end

-- Read our complete template for wb_usr.lua
local wbloc = "wb/wb_usr_template.lua"
local realwbloc = "wb/wb_usr.lua"
local wbf = io.open( wbloc, "rb" )
if not wbf then
  print( string.format( "Cannot open %s for reading", wbloc ) )
  return 1
end
local wbdata = wbf:read( "*a" )
wbf:close()
-- Check that our template has the required pattern
local pattern = "%$%$ARCH_PLATFORM%$%$"
if not wbdata:find( pattern ) then
  print( string.format( "$$arch_platform$$ not found in wb_usr_template.lua", modname ) )
  return 1
end
-- Open the actual wb_usr.lua in write mode
local realwbf = io.open( realwbloc, "wb" )
if not realwbf then
  print( string.format( "Cannot open %s for writing", realwbloc ) )
  return 1
end

-- Generate documentation for each module in turn
local fulldata = {}
for _, modname in pairs( arch_platform_components ) do
  local descfname = string.format( "arch_platform/arch_platform_%s.lua", modname )
  local res, err = build_file( descfname )
  if res then
    fulldata[ modname ] = res
    -- Write doc for each language
    for _, lang in pairs( languages ) do
      local fname = string.format( "%s/arch_platform_%s.html", lang, modname )
      local f = io.open( fname, "wb" )
      if not f then
        print( string.format( "Unable to open %s for writing", fname ) )
        return 1
      else
        f:write( res[ lang ].page )
        f:close()
        print( ( "Wrote %s" ):format( fname ) )
      end
    end
  else
    print( string.format( "Error processing module '%s': %s", modname, err ) )
    return 1
  end
  print ""
end 

-- Now it's finally time to get our wb/wb_usr.lua
local fullwb = ''
for _, modname in pairs( arch_platform_components ) do
  local wbstr = wb2str( fulldata, modname )
  fullwb = fullwb .. wbstr
end
-- Add beginning and end to fullwb
fullwb = string.format( '{ %s,\n  link = "arch_platform.html",\n  folder = \n  {\n%s  }\n}\n', alllangs( function( x ) return pi_tr[ x ] end ), fullwb )

-- Substitute our pattern and write everything back to disk
wbdata = wbdata:gsub( pattern, fullwb )
realwbf:write( wbdata )
realwbf:close()

print "All done, remember to run 'lua wb_build.lua' in the 'wb' directory to build your navigation tree"

