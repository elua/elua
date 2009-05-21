-- eLua doc builder tool 

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
  -- replace double "special chars" with "temps" for later usage
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

  -- process title
  if not title then
    return false, "title not found"
  end
  local page = header:format( title ) .. "\n\n"

  -- process overview
  if not overview then
    return false, "overview not found"
  end
  page = page .. '<a name="overview"><h3>Overview</h3></a>\n<p>' .. format_string( overview ) .. "</p>\n\n"

  -- process structures if needed
  if structures then
    page = page .. "<h3>Data structures, constants and types</h3>\n"
    for i = 1, #structures do
      local s = structures[ i ]
      if not s.text or not s.desc then
        return false, "structure without text or desc fields"
      end
      -- text/link
      if s.link then page = page .. string.format( '<a name="%s">', s.link ) end
      page = page .. "<p><pre><code>" .. format_string( s.text, true ) .. "</code></pre></p>"
      if s.link then page = page .. "</a>" end
      -- description
      page = page .. "\n<p>" .. format_string( s.desc ) .. "</p>\n\n"
    end 
  end

  -- process functions now
  if not funcs then
    return false, "funcs not found"
  end
  page = page .. "<h3>Functions</h3>\n"
  for i = 1, #funcs do
    local f = funcs[ i ]
    if not f.sig or not f.desc then
      return false, "function without sig or desc fields"
    end
    -- signature/link
    if f.link then page = page .. string.format( '<a name="%s">', f.link ) end
    page = page .. "<p><pre><code>" .. f.sig .. "</code></pre></p>"
    if f.link then page = page .. "</a>" end
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
  if auxdata then
    for i = 1, #auxdata do
      local a = auxdata[ i ]
      if not a.title or not a.desc then
        return false, "auxdata without title or desc"
      end
      if a.link then page = page .. string.format( '<a name="%s">', a.link ) end
      page = page .. "<h3>" .. a.title .. "</h3>"
      if a.link then page = page .. "</a>" end
      page = page .. "\n<p>" .. format_string( a.desc ) .. "</p>\n\n"
    end
  end

  -- footer
  page = page .. "</body></html>\n"
  return page 
end


local args = { ... }
if #args ~= 1 then
  print "Usage: builddoc <filename>"
  return 1
end

local page, res = build_file( args[ 1 ] )
if page then
  local fname = args[ 1 ]:gsub( "%.lua", "" ) .. ".html"
  local f = io.open( fname, "wb" )
  if not f then
    print( string.format( "Unable to open %s for writing", fname ) )
    return 1
  else
    f:write( page )
    f:close()
    print( ( "Wrote %s" ):format( fname ) )
  end
else
  print( string.format( "Error processing %s: %s", args[ 1 ], res ) )
  return 1
end

