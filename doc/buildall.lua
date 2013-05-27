package.path = package.path .. ";../utils/?.lua;"

require "lfs"
require "eluadoc"
require "md5"
local utils = require "utils"

-- Uncomment this when generating offline docs
local is_offline = true

-- Languages in the system
-- NOTE: "en" must ALWAYS be the first entry in this array!
-- NOTE: all languages must be given in lowercase only!

-- ## Obs: PT going offline in July 2010 for lack of support. 
--         We hope to offer it again and I'll keep maintaining (offline) what I can.
--languages = { "en", "pt" } 
languages = { "en" }

-- Reverse lookup (language to idx) dictionary
local langidx = {}
for k, v in ipairs( languages ) do
  langidx[ v ] = k
end

local sf = string.format
local cache_invalid = false

-------------------------------------------------------------------------------
-- Indexes into our menu table (defined in docdata.lua)
name_idx, link_idx, submenu_idx , title_idx = 1, 2, 3, 4

-------------------------------------------------------------------------------
-- "getstr" support (return strings in different languages)
-- If defaults to english (but gives a warning) if the string isn't found in the given language

-- This table keeps the strings we already emitted warnings for
-- After all, we don't want to drive the user crazy
local warned = {}
function getstr( str, lang )
  -- Get the language index from langidx
  local idx = langidx[ lang ]
  if not idx then error( string.format( "Invalid language %s", lang ) ) end
  -- Look from the string in the "translations" table
  local where
  for _, v in ipairs( translations ) do
    if v[ 1 ] == str then
      where = v
      break
    end
  end
  if not where then error( string.format( "String %s not found in translations" , str ) ) end
  -- Try to return the value in the specified language
  -- If not possible, return the value in english, but issue a warning first
  local res = where[ idx ]
  if not res then
    res = where[ 1 ]
    if not warned[ str ] then
      print( string.format( "*** WARNING: translation for '%s' in language '%s' not found!", str, lang ) )
      warned[ str ] = true
    end
  end
  return res
end

-------------------------------------------------------------------------------
-- Generic helper functions

-- Remove anchor from a link of the form a/b.../baselink.html#anchor
local function get_base_link( name )
  return ( name:gsub( "#.*", "" ) )
end

-- Get the menu field for a given item and language
-- Returns the english name is the field for the specified language can't be found
local function get_menu_field( menuitem, lang, fieldidx )
  if not menuitem[ fieldidx ] then
    return nil
  else
    if type( menuitem[ fieldidx ] ) == "string" then
      return menuitem[ fieldidx ]
    else
      local lidx = langidx[ lang ]
      return menuitem[ fieldidx ][ lidx ] or menuitem[ fieldidx ][ 1 ]
    end
  end
end

-- Get the menu name for a given menu item and a language
-- Returns the english name if the name for the specified language can't be found
local function get_menu_name( menuitem, lang )
  return get_menu_field( menuitem, lang, name_idx )
end

-- Get the link for a given menu item and a language
-- Returns the english name if the name for the specified language can't be found
-- If the link field doesn't exists, the name is returned instead
local function get_menu_title( menuitem, lang )
  return "" .. ( get_menu_field( menuitem, lang, title_idx ) or  get_menu_field( menuitem, lang, name_idx ) )
end

-- Set "print" to print indented (with 2 spaces)
local oldprint
local function indent_print()
  oldprint = print
  print = function( ... ) io.write( "  " ); oldprint( ... ) end
end

-- Restore the "regular" print function
local function regular_print()
  print = oldprint
end

-------------------------------------------------------------------------------
-- File/directory operations helpers

-- Copy the given file to the 'dest' directory
-- Doesn't do error checking
local function copy_file( fname, dst ) 
  local destname = fname
  if fname:find( "/" ) then
    -- Get only the filename from the path
    local sidx
    for f = #fname, 1, -1 do
      if fname:sub( f, f ) == "/" then 
        sidx = f
        break 
      end
    end
    destname = fname:sub( sidx + 1 )
  end
  local fsrc = io.open( fname, "rb" )
  local fdst = io.open( string.format( "%s/%s", dst, destname ), "wb" )
  local data = fsrc:read( "*a" )
  fdst:write( data )
  fsrc:close()
  fdst:close()
end

-- Copy the 'src' directory to the 'dst' directory, going recursively through
-- its content. Doesn't do error checking.
local function copy_dir_rec( src, dst )
  for f in lfs.dir( src ) do
    local oldf = string.format( "%s/%s", src, f )
    local attrs = lfs.attributes( oldf )
    if attrs.mode == 'directory' and f ~= "." and f ~= ".." and f ~= ".svn" and f ~= ".git" then
      local newdir = string.format( "%s/%s", dst, f )
      lfs.mkdir( newdir )
      copy_dir_rec( oldf, newdir )
    elseif attrs.mode == 'file' then
      copy_file( oldf, dst )
    end
  end
end

-- Copy a directory to another directory
local function copy_dir( src, dst )
  local newdir = string.format( "%s/%s", dst, src )
  lfs.mkdir( newdir )
  copy_dir_rec( src, newdir )
end

-------------------------------------------------------------------------------
-- Cache helpers

local function read_md5( filename )
  local fullname = string.format( "cache/%s.cache", filename )
  local f = io.open( fullname, "rb" )
  if not f then return "" end
  local d = f:read( "*a" )
  f:close()
  return d
end

local function write_md5( filename, d )
  local fullname = string.format( "cache/%s.cache", filename )
  local f = io.open( fullname, "wb" )
  if not f then return false end
  f:write( d )
  f:close()
  return true
end

local function file_md5( filename )
  local f = io.open( filename, "rb" )
  if not f then return "" end
  local d = f:read( "*a" )
  f:close()
  return md5.sumhexa( d ) 
end

-------------------------------------------------------------------------------
-- Table utils (from http://lua-users.org/wiki/TableUtils)

function table.val_to_str( v )
  if "string" == type( v ) then
    v = string.gsub( v, "\n", "\\n" )
    if string.match( string.gsub(v,"[^'\"]",""), '^"+$' ) then
      return "'" .. v .. "'"
    end
    return '"' .. string.gsub(v,'"', '\\"' ) .. '"'
  else
    return "table" == type( v ) and table.tostring( v ) or tostring( v )
  end
end

function table.key_to_str ( k )
  if "string" == type( k ) and string.match( k, "^[_%a][_%a%d]*$" ) then
    return k
  else
    return "[" .. table.val_to_str( k ) .. "]"
  end
end

function table.tostring( tbl )
  local result, done = {}, {}
  for k, v in ipairs( tbl ) do
    table.insert( result, table.val_to_str( v ) )
    done[ k ] = true
  end
  for k, v in pairs( tbl ) do
    if not done[ k ] then
      table.insert( result,
        table.key_to_str( k ) .. "=" .. table.val_to_str( v ) )
    end
  end
  return "{" .. table.concat( result, "," ) .. "}"
end


-------------------------------------------------------------------------------
-- Build the list of files that must be processed starting from the menu data

-- Traverse a second (or higher) level menu and add relevant information to flist
local function traverse_list( item, parentid, flist )
  if not item[ link_idx ] then return end
  local base = get_base_link( item[ link_idx ] )
  if base ~= "" and not flist[ base ] then
    flist[ base ] = { parentid = parentid, item = item } 
  end
  if item[ submenu_idx ] then
    for i = 1, #item[ submenu_idx ] do
      traverse_list( item[ submenu_idx ][ i ], parentid, flist )
    end
  end
end

-- Iterate over the menu list, building the list of files that must be 
-- processed by the doc generator. Returns a dictionary with list, parent_id
-- pairs where parent_id is the parent menu of link in themenu
local function get_file_list()
  local flist = {}
  for i = 1, #themenu do
    traverse_list( themenu[ i ], i, flist )
  end
  return flist
end

-- Returns true if the given string begins with the given substring, false otherwise
-- The comparation is case-insensitive
local function beginswith( str, prefix )
  return str:sub( 1, #prefix ):lower() == prefix:lower()
end

-------------------------------------------------------------------------------
-- Build the navigation data for a given page

-- Helper function: format a link starting from language and link
-- Links marked as "#" ("null" links) are left alone
-- Links that begin with "http(s)://" are unchanged

local function get_link( lang, link )
  if link == "#" then
    return "#"
  elseif link:find( "https?://" ) == 1 then
    return link
  else
    return string.format( "%s_%s", lang, link )
  end
end

-- Helper for gen_html_nav: generate the submenu(s) for a given top level menu item
local function gen_submenus( item, lang, level )
  level = level or 1
  local data = ''
  local lidx = langidx[ lang ]
  local arrptr = ''
  for i = 1, #item do
    local l = item[ i ]
    if l[ submenu_idx ] then
      local link = l[ link_idx ]
      local string_item = (link == "" or not link) and '<li>%s<span class="folder">%s%s</span>\n' or '<li> <a href="%s">%s%s</a>\n'
      data = data .. string.rep( " ", level * 2 + 8 ) .. string.format( string_item, get_link( lang, l[ link_idx ] ), arrptr, get_menu_name( l, lang ) )
      data = data .. string.rep( " ", level * 2 + 8 ) .. "<ul>\n"
      data = data .. gen_submenus( l[ submenu_idx ], lang, level + 1 )
      data = data .. string.rep( " ", level * 2 + 8 ) .. "</ul></li>\n"
    else
      if get_menu_name( l, lang ) then
        local link = l[ link_idx ]
        local string_item = (link == "" or not link) and '<li>%s<span class="folder">%s</span></li>\n' or '<li> <a href="%s">%s</a></li>\n'
        data = data .. string.rep( " ", level * 2 + 8 ) .. string.format( string_item, get_link( lang, l[ link_idx ] ), get_menu_name( l, lang ) )
      end
    end
  end
  return data
end

-- Generate the HTML menu structure for the given language and parentid
-- If "is_offline" is true, don't generate links to the counter and the BerliOS logo
local function gen_html_nav( parentid, lang )
  local htmlstr = [[
    <ul id="menu-nav">
]]
  local lidx = langidx[ lang ]
  for i = 1, #themenu do
    local menudata = ""
    local imginsert = ""
    local styledef =  i == #themenu and ' style="border-bottom-width: 0"' or ""
    local link = themenu[ i ][ link_idx ]
    local name = get_menu_name( themenu[ i ], lang )
    if not link then
      htmlstr = htmlstr .. string.format('      <li class="sep"%s>%s</li>\n', styledef, name )
    else
      local relname = string.gsub( string.gsub( string.format( "s_%d_%s", i, string.lower( get_base_link( link ) ) ), "%s", "_" ), "%.html", "" )
      -- If we have a submenu, update the HTML menu content part
      if themenu[ i ][ submenu_idx ] then
        menudata = string.format( [[
        <ul id="%s">      
          %s
        </ul>
        ]], relname, string.sub( gen_submenus( themenu[ i ][ submenu_idx ], lang ), 1, -2 ) )
        imginsert = ''
      end
      if name then
        if i == parentid then
          -- If this is the parent, use a special style for it (<a class="current"> or <li class="current">, depending on the item type)
          if themenu[ i ][ submenu_idx ] then
            local string_item = (link == "" or not link) and '      <li><span class="folder" %s rel="%s"%s>%s%s</span>\n%s      </li>\n' or '      <li><a class="current" href="%s" rel="%s"%s>%s%s</a>\n%s      </li>\n'
            htmlstr = htmlstr .. string.format(string_item, get_link( lang, link ), relname, styledef, imginsert, name, menudata )
          else
            htmlstr = htmlstr .. string.format('      <li class="current"%s>%s%s\n%s      </li>\n', styledef, imginsert, name, menudata )
          end
        else
          local submenustr = themenu[ i ][ submenu_idx ] and string.format( ' rel="%s"', relname ) or ""
          local string_item = (link == "" or not link) and '      <li> <span class="folder" %s %s%s>%s%s</span>\n%s     </li>\n' or '      <li> <a href="%s"%s%s>%s%s</a>\n%s     </li>\n'
          htmlstr = htmlstr .. string.format(string_item, get_link( lang, link ), submenustr, styledef, imginsert, name, menudata )
        end
      end
    end
  end
  offline_data = not is_offline and [[
  <p style="margin-left: 35px;"><a href="http://www.pax.com/free-counters.html"><img src="http://counter.pax.com/counter/image?counter=ctr-zsg80nnmqt" alt="Free Hit Counter" style="border: 0;" /></a></p>
  <p style="margin-left: 18px;"><a href="http://developer.berlios.de" title="BerliOS Developer"> <img src="http://developer.berlios.de/bslogo.php?group_id=9919" width="124px" height="32px" style="border: 0;" alt="BerliOS Developer Logo" /></a></p>
]] or ""
  htmlstr = htmlstr .. string.format( [[
    </ul>  
%s
]], offline_data )
  return htmlstr
end

-- Helper function: replace local links with links prefixed by language
local function language_for_links( lang, orig )
    -- Iterate through all the links in the document and change the local ones with
    -- the correct language option
    orig = orig:gsub( [==[<a href=["'](.-)["']>]==], function( link )
      if beginswith( link, "#" ) or beginswith( link, "http://" ) or beginswith( link, "https://" ) or beginswith( link, "ftp://" ) then
        return string.format( '<a href="%s">', link )
      else
        return string.format( '<a href="%s_%s">', lang, link )
      end
    end )
    return orig
end

-------------------------------------------------------------------------------
-- Build the logo for a given language

local function gen_logo( fname, lang )
  local numl = #languages
  local langdata = ''
  for i = 1, numl do
    local crtlang = languages[ i ]
    local hlang = crtlang:sub( 1, 1 ):upper() .. crtlang:sub( 2, -1 )
    if lang:lower() == crtlang:lower() then 
      langdata = langdata .. string.format('          <td align="center"><h6 class="selected"><img src="images/%s.jpg" alt="%s" style="border: 0;" /></h6></td>\n', hlang, crtlang )
    else
      langdata = langdata .. string.format('          <td align="center"><h6><a href="%s_%s" class="lang"><img src="images/%s.jpg" alt="%s" style="border: 0;" /></a></h6></td>\n', crtlang:lower(), fname, hlang, crtlang )
    end
  end
return string.format( [[

]], getstr( "eLua - Embedded Lua", lang ), numl + 1, getstr( "Search", lang ), getstr( "Language", lang ), langdata:sub( 1, -2 ) )
end

-------------------------------------------------------------------------------
-- Generate an actual HTML page starting from a template
-- Replace the $$HEADER$$ and $$FOOTER$$ with proper data
local function gen_html_page( fname, lang )
  local entry = flist[ fname ]
  local parentid = entry.parentid
  local item = entry.item

  -- Open and read file
  local fullname = string.format( "%s/%s", lang, fname )
  local f = io.open( fullname, "rb" )
  if not f then
    fullname = fullname:gsub( "%.html", "%.txt" )
    f = io.open( fullname, "rb" )
    if not f then
      return nil, string.format( "Error opening %s", fullname )
    end
  end
  local orig = f:read( "*a" )
  f:close()
  
  -- Check cache
  local cfilename = string.format( "%s_%s", lang, fname )
  local oldsum = read_md5( cfilename )
  local crtsum = md5.sumhexa( orig )
  if oldsum == crtsum then
    if not cache_invalid then 
      return nil, "#cached#"
    end
  else
    write_md5( cfilename, crtsum )
  end
      
  local asciimode = fullname:find( "%.txt" )

  -- Check the presence of $$HEADER$$ and $$FOOTER$$
  if not orig:find( "%$%$HEADER%$%$" ) or not orig:find( "%$%$FOOTER%$%$" ) then
    return nil, string.format( "%s not formated properly ($$HEADER$$ or $$FOOTER$$ not found)", fullname )
  end

  if not asciimode then
    print ""
    -- Anticipate some common errors and fix them directly
    orig = orig:gsub( "<br>", "<br />" )
    orig = orig:gsub( '(<a name=["\'][^\'"]-["\']>)([^\n]-)</a>%s-\n', function( anchor, data )
      return anchor:gsub( ">", " />" ) .. data .. "\n"
    end )
    orig = orig:gsub( '<p><pre><code>(.-)</code></pre></p>', "<pre><code>%1</code></pre>" )
    orig = orig:gsub( 'target="_blank"', "" )
  else
    print( "(AsciiDoc mode)" )
    -- Call "asciidoc" to generate the actual HTML
    local tempname = fullname .. '.temp' 
    os.execute( sf( "asciidoc -s -a icons -a 'newline=\\n' -b xhtml11 -o %s %s", tempname, fullname ) )
    local resfile = io.open( tempname, "rb" )
    if not resfile then
      return nil, sf( "Unable to find the AsciiDoc generated file %s", tempname )
    end
    orig = resfile:read( "*a" )
    resfile:close()
    orig = "$$HEADER$$\n" .. orig .. "$$FOOTER$$\n"
    os.remove( tempname )
  end
  -- Replace local links with language-dependent links
  orig = language_for_links( lang, orig )
  local main_header_title = "eLua Doc"
  
  -- Generate actual data
  local header = [=[
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html>
 <head>
  <meta charset="utf-8">
	<meta name="subject" content="eLua - Lua for the embedded world" />
  <meta name="Description" content="eLua stands for Embedded Lua and the project aims to offer the full set of features of the Lua Programming Language to the embedded world." />
  <meta name="Keywords" content="eLua, lua, embedded, ARM, Cortex-M3, AVR32, ARM7TDMI, microcontroller, mcu, programming, electronics, tools, development" />
  
	<script src="js/jquery.js" type="text/javascript"></script>
	<script src="js/jquery.cookie.js" type="text/javascript"></script>
	<script src="js/jquery.treeview.js" type="text/javascript"></script>

  <link href="css/style1.css" rel="stylesheet" type="text/css"> 
  <link rel="stylesheet" href="css/jquery.treeview.css" />
  <title>eLua - ]=]..get_menu_title( item, lang )..[=[</title>
  
  <link REL="SHORTCUT ICON" HREF="images/eLua_16x16.ico">
  <script type="text/javascript">

  var _gaq = _gaq || [];
  _gaq.push(['_setAccount', 'UA-11834941-1']);
  _gaq.push(['_setDomainName', 'eluaproject.net']);
  _gaq.push(['_trackPageview']);

  (function() {
    var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
    ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
    var s = document.getElementsByTagName('script')[0]; s.parentNode.insertBefore(ga, s);
  })();

  </script>
 </head>
  
 <body id="interna_2">
  <!--header-->
 <div id="header">
  	<div class="wrapper">
    
  	<form method="get" action="http://www.google.com/search" class="searchform" style="margin-top:62px;">
        <input type="hidden" name="ie" value="utf-8" />
        <input type="hidden" name="oe" value="utf-8" />
        <input type="hidden" name="domains" value="http://www.eluaproject.net" />
        <input type="hidden" name="sitesearch" value="http://www.eluaproject.net" />
				<input class="searchfield" type="text"  name="q" maxlength="255" value="Search..." onfocus="if (this.value == 'Search...') {this.value = '';}" onblur="if (this.value == '') {this.value = 'Search...';}" />
				<input class="searchbutton" type="submit" value="Go" />
    </form>
	
  		<h1><a href="en_index.html" title="eLua Project"><img src="images/logo_elua_menor.png" alt="eLua Project"/></a></h1>
  		
  		<!--navegação-->
  		<div id="nav_top">
  			<ul class="sections">
          <li><a href="http://www.eluaproject.net" >Site</a></li>
  				<li><a href="http://www.eluaproject.net/get-started/downloads" >Downloads</a></li>
  				<li><a href="http://www.eluaproject.net/doc" class="selected">Doc</a></li>
  				<li><a href="http://wiki.eluaproject.net" >Wiki</a></li>
  				<li><a href="http://builder.eluaproject.net" >Builder</a></li>
  				<li><a href="http://tracker.eluaproject.net" >Tracker</a></li>
  				<li><a href="http://www.github.com/elua" >Repos</a></li>
  			</ul>
  		</div>
  		<!--navegação-->
  		
  		<!--chamada-->
  		<div class="chamada_elua">
  				<h2>]=]..main_header_title..[=[</h2>
			</div>
  			<!--chamada-->
	
  </div>
  <!--header-->
</div>  

<div id="article">
		<div class="wrapper">
		
			<div class="section_menu">
			<!--conteudo Esquerda-->
		]=] ..gen_logo( fname, lang ) .. "\n"..gen_html_nav( parentid, lang )
		..
	[=[
			<!--conteudo esquerda-->
			</div>

  			<div class="section_conteudo section_size">
			<!--conteudo-->
      <h1>]=]..get_menu_title( item, lang )..[=[ </h1>
      ]=] 


  
  local footer =  [[
			<!--conteudo-->
			</div>
			
		
		</div>
	<br/>
	</div>

  <!--footer-->
  <div id="footer">
  	<div class="wrapper">
       <br/>
        <span class="rodape">© 2011 eLua Project. All rights reserved.</span> 
  	</div>
  	<br/>
  </div>
  <!--footer-->

<script type="text/javascript">
	$("#menu-nav").treeview({
		persist: "location",
		collapsed: true,
		unique: true
	});
</script>

</body>
</html>
]]
  orig = orig:gsub( "%$%$HEADER%$%$", header )
  orig = orig:gsub( "%$%$FOOTER%$%$", footer )
  return orig
end

-------------------------------------------------------------------------------
-- Documentation generator

-- Helper function: iterate through the menu and replace automatically generated content
local function replace_auto_content( automenus, item )
  if type( item[ submenu_idx ] ) == "string" then
    local r = automenus[ item[ submenu_idx ] ]
    if not r then
      return string.format( "Autogenerated menu '%s' not found", item[ submenu_idx ] )
    else
      print( string.format( "Replaced autogenerated menu '%s'", item[ submenu_idx ] ) )
      item[ submenu_idx ] = r
    end
  elseif type( item[ submenu_idx ] ) == "table" then
    for i = 1, #item[ submenu_idx ] do
      replace_auto_content( automenus, item[ submenu_idx ][ i ] )
    end
  end
end

-- Argument check
local args = { ... }
local destdir = "dist"
local destdiridx = 1
if #args > 2 then
  print "Usage: buildall.lua [destdir] [-online] [-clean]"
  print "Use -online to generate online documentation (includes BerliOS logo and counter)"
  print "Use -clean to clear the cache and generate clean documentation"
  return
end
local cleancache = false
for i = 1, #args do
  if args[ i ] == "-online" then
    is_offline = false
  elseif args[ i ] == "-clean" then
    cleancache = true
  else 
    destdir = args[ i ]
  end
end
print( sf( "Using '%s' as the destination directory", destdir ) );

-- Read the documentation data
themenu, translations, fixed = dofile( "docdata.lua" )
if not themenu or not translations or not fixed then
  print "docdata.lua doesn't return the proper data, aborting."
  return
end

-- Add the content generated from eluadoc to our menu(s)
print "Generating HTML documentation..."
indent_print()
local automenus, genfiles = eluadoc.gen_html_doc()
if not automenus then return end
regular_print()
-- Replace content generated by gen_html_doc in the menu
for i = 1, #themenu do
  local replerr = replace_auto_content( automenus, themenu[ i ] )
  if replerr then
   print( replerr )
   return
 end
end
print( "done" )

-- If the destination directory doesn't exist, create it
-- If it exists, remove it
local attr = lfs.attributes( destdir )
if not attr then
  if not lfs.mkdir( destdir ) then
    print( string.format( "Unable to create directory %s", destdir ) )
    return
  end
else
  if attr.mode ~= "directory" then
    print( string.format( "%s is not a directory", destdir ) )
    return
  end
  utils.rmdir_rec( destdir )
  lfs.mkdir( destdir )
end

-- If the cache must be cleared, do it now
if cleancache then
  local attr = lfs.attributes( 'cache' )
  if attr then
    if attr.mode ~= "directory" then
      print( "'cache' is not a directory" )
      return
    end
    utils.rmdir_rec( 'cache' )
    lfs.mkdir( 'cache' )
  end
end

-- Create the cache directory if it doesn't exist
local attr = lfs.attributes( 'cache' )
if not attr then
  if not lfs.mkdir( 'cache' ) then
    print( "Unable to create cache directory" )
    return
  end
end  

-- Set the global "cache invalid" flag
-- It is set to 'true' if the content of docdata.lua changes
local crtdocsum = md5.sumhexa( table.tostring( themenu ) )
local oldsum = read_md5( "docdata" )
cache_invalid = crtdocsum ~= oldsum
if cache_invalid then 
  write_md5( "docdata", crtdocsum )
  print "Cache invalidated" 
end

print "\nProcessing HTML templates..."
indent_print()
flist = get_file_list()
for _, lang in ipairs( languages ) do
  for fname, entry in pairs( flist ) do
    if fname:find( "https?://" ) ~= 1 then -- not a filename but a direct link
      io.write( string.format( "Processing %s %s...", fname, entry.item[ name_idx ] and "" or "(hidden entry)" ) )
      local res, err = gen_html_page( fname, lang )
      if err == "#cached#" then
        -- This file is already in the cache
        print( " (cached)" )         
      elseif not res then
        print( "***" .. err ) 
      else
        local g = io.open( string.format( "cache/%s_%s", lang, fname ), "wb" )
        if not g then
          print( string.format( "Unable to open %s for writing", fname ) )
        else
          g:write( res )
          g:close()
        end
      end
      -- Copy file from cache to destination directory
      local srcf = io.open( string.format( "cache/%s_%s", lang, fname ), "rb" )
      local destf = io.open( string.format( "%s/%s_%s", destdir, lang, fname ), "wb" )
      if not srcf or not destf then
        print "Unable to copy file from cache to dist"
        return
      end
      local content = srcf:read( "*a" )
      destf:write( content )
      srcf:close()
      destf:close()    
    end
  end
end
regular_print()
print "done"

-- Now copy the fixed content in the documentation directory
print "\nCopying fixed content ..."
indent_print()
for _, v in ipairs( fixed ) do
  print( string.format( "Copying %s", v ) )
  if v:sub( -1 ) == "/" then 
    copy_dir( v, destdir )
  else
    copy_file( v, destdir )
  end
end
regular_print()
print "done"

-- And delete the files generated by eluadoc
print "\nCleaning up files generated by eluadoc..."
indent_print()
for _, v in pairs( genfiles ) do 
  print( string.format( "Deleting %s...", v ) )
  os.remove( v )
end
regular_print()
print "done"

print( string.format( "\nEnjoy your documentation in %s :)", destdir ) )

