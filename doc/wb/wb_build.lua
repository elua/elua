
dofile("wb_usr.lua")

lngCount = nil
lngSuffix = nil
lngIndex = nil
lngNext = nil
linkCount = 1

function readFile(filename)
  local file = io.open(filename)
  local text = file:read("*a")
  file:close()
  return text
end

function writeFile(filename, text)
  local file = io.open(filename, "w")
  file:write(text)
  file:close()
end

-- #####################################################################

htmlFiles = {}

function addHtmlFile(v)
  if v then
    -- only up to "#"
    local j = string.find(v, "#")
    if j then
      f = string.sub(v, 0, j-1)
    else
      f = v
    end
  
    htmlFiles[f] = f
  end
end

-- #####################################################################

function writeIndexFile()
  print("Writing \"../index"..lngSuffix..".html\".")
  
  local wb_index = readFile("template_index.html")
  
  wb_index = string.gsub(wb_index, "WB_TITLE", wb_usr.messages[lngIndex].title)
  wb_index = string.gsub(wb_index, "WB_START_SIZE", wb_usr.start_size)
  wb_index = string.gsub(wb_index, "WB_START_PAGE", lngIndex.."/"..wb_usr.tree.link)
  if (lngCount > 1) then
    wb_index = string.gsub(wb_index, "WB_LNG", lngSuffix)
  else
    wb_index = string.gsub(wb_index, "WB_LNG", "")
  end
  
  writeFile("../index"..lngSuffix..".html", wb_index)
end

-- #####################################################################

function writeTitleFile()
  print("Writing \"../wb_title"..lngSuffix..".html\".")
  
  local wb_title = readFile("template_wb_title.html")
  
  wb_title = string.gsub(wb_title, "WB_BAR_TITLE", wb_usr.messages[lngIndex].bar_title)
  wb_title = string.gsub(wb_title, "WB_TITLE_BGCOLOR", wb_usr.title_bgcolor)
  wb_title = string.gsub(wb_title, "WB_SEARCH_LINK", wb_usr.search_link)
  wb_title = string.gsub(wb_title, "WB_COPYRIGHT_LINK", wb_usr.copyright_link)
  wb_title = string.gsub(wb_title, "WB_COPYRIGHT_NAME", wb_usr.copyright_name)
  wb_title = string.gsub(wb_title, "WB_CONTACT", wb_usr.contact)
  
  if (lngCount > 1) then
    wb_title = string.gsub(wb_title, "WB_LNG", lngSuffix)
  else
    wb_title = string.gsub(wb_title, "WB_LNG", "")
  end
  
  writeFile("../wb_title"..lngSuffix..".html", wb_title)
end

-- #####################################################################

function writeIndent(file, level)
  -- base identation             
  file:write("    ")       
  
  for i = 1, level*2, 1 do
    file:write(" ")
  end
end

function getNodeName(node)
  local name = nil
  if (node.name[lngIndex]) then
    name = node.name[lngIndex]
  else
    name = node.name["nl"]
  end

  if not name then 
    error("Name not found.")
  end
  
  return name
end

function writeNode(file, node, opened, level, folder_index, folder_suffix, node_suffix, child_prefix)
  if (node.folder) then -- folder
    -- box image
    writeIndent(file, level)
    file:write("<p>")
    
    folder_suffix = folder_suffix .. "." .. folder_index
                                         
    file:write(child_prefix.."<img name=\"imgfolder"..folder_suffix.."\" ")
      
    if (opened) then
      file:write("src=\"wb_img/minusnode"..node_suffix..".png\" ")
    else
      file:write("src=\"wb_img/plusnode"..node_suffix..".png\" ")
    end
    
    file:write("onclick=\"toggleFolder('folder"..folder_suffix.."')\">")
    
    if (node.link) then
      file:write("<a name=\"link"..linkCount.."folder"..folder_suffix.."\" class=\"el\" href=\""..lngIndex.."/"..node.link.."\">"..getNodeName(node).."</a>")
      addHtmlFile(node.link)
      linkCount = linkCount + 1
    else
      file:write("  "..getNodeName(node))
    end
    
    file:write("</p>\n")
                
    -- folder div            
    writeIndent(file, level)
    if (opened) then
      file:write("<div id=\"folder"..folder_suffix.."\" style=\"display:block\">\n")
    else
      file:write("<div id=\"folder"..folder_suffix.."\">\n")
    end
  
    local n = #(node.folder)
    local next_folder_index = 0
    local next_node_suffix = ""
    local next_child_prefix = "<img src=\"wb_img/vertline.png\">"
    if (node_suffix == "last") then
      next_child_prefix = "<img src=\"wb_img/blank.png\">"
    end
    for i = 1, n, 1 do
      if (i == n) then
        next_node_suffix = "last"
      end
      if (node.folder[i].folder) then
        next_folder_index = next_folder_index + 1
      end
      writeNode(file, node.folder[i], false, level+1, next_folder_index, folder_suffix, next_node_suffix, child_prefix..next_child_prefix)
    end
      
    writeIndent(file, level)
    file:write("</div>\n")
  else -- leaf
    if (node.link and node.link ~= "") then  -- normal leaf
      writeIndent(file, level)
      file:write("<p>"..child_prefix.."<img src=\"wb_img/node"..node_suffix..".png\"><a class=\"el\" name=\"link"..linkCount.."folder"..folder_suffix.."\" href=\""..lngIndex.."/"..node.link.."\">"..getNodeName(node).."</a></p>\n")
      addHtmlFile(node.link)
      linkCount = linkCount + 1
    else  -- separator leaf
      writeIndent(file, level)
      file:write("<p class=\"sep\">")
      
      local sep_child_prefix = string.gsub(child_prefix, "/vertline", "/sepvertline")
      sep_child_prefix = string.gsub(sep_child_prefix, "/blank", "/sepblank")
      
      file:write(sep_child_prefix.."<img src=\"wb_img/sepnode.png\"></p>\n")
    end
  end  
end

function writeTree(file)
  -- root node              
  file:write("    <h3><a name=\"link0folder.0\" class=\"el\" href=\""..lngIndex.."/"..wb_usr.tree.link.."\">"..getNodeName(wb_usr.tree).."</a></h3>\n")
  addHtmlFile(wb_usr.tree.link)
                       
  local folder = wb_usr.tree.folder                     
  local n = #folder
  local node_suffix = "" 
  local folder_index = 0
  for i = 1, n, 1 do
    if (i == n) then
      node_suffix = "last"
    end                        
    if (folder[i].folder) then
      folder_index = folder_index + 1
    end
    if (i == 1 and wb_usr.start_open) then
      writeNode(file, folder[i], true, 1, folder_index, "", node_suffix, "")
    else
      writeNode(file, folder[i], false, 1, folder_index, "", node_suffix, "")
    end
  end
end

function writeTreeFile()
  print("Writing \"../wb_tree"..lngSuffix..".html\".")
  
  local file = io.open("../wb_tree"..lngSuffix..".html", "w")
   
  -- Write Header 
  local wb_tree = readFile("template_wb_tree.html")
  file:write(wb_tree)
                   
  -- Write Tree Nodes and Leafs
  writeTree(file)
                               
  -- Write Footer                             
  file:write("  </div>\n")
  file:write("</body>\n")
  file:write("</html>\n")
  
  file:close()
end
              
-- #####################################################################

lngMessages =
{               
  search= {
    en= "Simple Search",
    pt= "Busca Simples", 
    es= "Busca Simples",
  },
  exp_all= {
    en= "Expand All Nodes",
    pt= "Expande Todos os Nós", 
    es= "Ensanchar Todos Nodos",
  },
  cont_all= {
    en= "Contract All Nodes",
    pt= "Contrai Todos os Nós",
    es= "Contrato Todos Nodos",
  },
  sync= {
    en= "Sync Tree with Contents",
    pt= "Sincroniza Árvore com Conteúdo",
    es= "Sincroniza Árbol con el Contenido",
  },
  lang= {
    en= "Switch Language",
    pt= "Troca Idioma",
    es= "Cambie Idioma",
  },
  next= {
    en= "Next Link",
    pt= "Próximo Link",
    es= "Próximo Link",
  },
  prev= {
    en= "Previous Link",
    pt= "Link Anterior",
    es= "Link Anterior",
  },
} 

function writeBarFile()
  print("Writing \"../wb_bar"..lngSuffix..".html\".")
  
  local file = io.open("../wb_bar"..lngSuffix..".html", "w")
   
  local wb_bar = readFile("template_wb_bar.html")
  
  wb_bar = string.gsub(wb_bar, "WB_EXPALL_ALT", lngMessages.exp_all[lngIndex])
  wb_bar = string.gsub(wb_bar, "WB_CONTALL_ALT", lngMessages.cont_all[lngIndex])
  wb_bar = string.gsub(wb_bar, "WB_SYNC_ALT", lngMessages.sync[lngIndex])
  wb_bar = string.gsub(wb_bar, "WB_NEXT_ALT", lngMessages.next[lngIndex])
  wb_bar = string.gsub(wb_bar, "WB_PREV_ALT", lngMessages.prev[lngIndex])
  
  if (lngCount > 1) then                
    local lng_button = "<img src=\"wb_img/barlineright.png\">"
    lng_button = lng_button .. "<a target=\"_top\" href=\"index_"..lngNext..".html\"><img alt=\""..lngMessages.lang[lngIndex].."\" src=\"wb_img/lng"..lngSuffix..".png\" onmouseover=\"this.src='wb_img/lng"..lngSuffix.."_over.png'\" onmouseout=\"this.src='wb_img/lng"..lngSuffix..".png'\" style=\"border-width: 0px\"></a>"
    wb_bar = string.gsub(wb_bar, "WB_LNG_BUTTON", lng_button)
  else
    wb_bar = string.gsub(wb_bar, "WB_LNG_BUTTON", "")
  end
  
  file:write(wb_bar)
  file:close()
end

-- #####################################################################

function writeSearchFile()
  print("Writing \"../ssSearch"..lngSuffix..".html\".")
  
  local file = io.open("../ssSearch"..lngSuffix..".html", "w")
   
  local wb_search = readFile("template_ssSearch.html")
  
  wb_search = string.gsub(wb_search, "WB_SEARCH", lngMessages.search[lngIndex])
  
  if (lngCount > 1) then                
    wb_search = string.gsub(wb_search, "WB_LNG", lngSuffix)
  else
    wb_search = string.gsub(wb_search, "WB_LNG", "")
  end
  
  file:write(wb_search)
  file:close()
end
              
function writeSearchIndexFile()
  print("Writing \"../wb_search"..lngSuffix..".txt\".")
  
  local file = io.open("../wb_search"..lngSuffix..".txt", "w")
  
  local v = next(htmlFiles, nil)
  while v ~= nil do
    file:write(lngIndex.."/"..v.."\n")
    v = next(htmlFiles, v)
  end
  
  file:close()
end
              
-- #####################################################################

lngCount = 0
local first_name = nil
local prev_elem = nil
for name, elem in pairs(wb_usr.messages) do 
  if (lngCount == 0) then
    first_name = name
  end
  lngCount = lngCount + 1
  if (prev_elem) then
    prev_elem.next = name
  end
  prev_elem = elem
end
prev_elem.next = first_name
  
print("Building...")

for name, elem in pairs(wb_usr.messages) do 
  lngIndex = name
  lngNext = elem.next
  
  if (lngCount > 1) then
    lngSuffix = "_"..lngIndex
  else
    lngSuffix = ""
  end
  
  writeIndexFile()
  writeTitleFile()
  writeBarFile()
  writeTreeFile()
  writeSearchFile()            
  writeSearchIndexFile()
end

print("Done.")
