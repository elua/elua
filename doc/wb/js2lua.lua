-- Convert JavaScript into Lua

if (not arg[1]) then
  error("Javascript filename must be passed as a parameter.")
end
if (not arg[2]) then
  error("Lua filename must be passed as a parameter.")
end
      
print("Converting...")

local js_filename = arg[1]
local file = io.open(js_filename)
local wb_lua = file:read("*a")
file:close()

wb_lua = string.gsub(wb_lua, ":", "=")
wb_lua = string.gsub(wb_lua, "http=", "http:")
wb_lua = string.gsub(wb_lua, ";", " ")
wb_lua = string.gsub(wb_lua, "%[", "{")
wb_lua = string.gsub(wb_lua, "%]", "}")

local lua_filename = arg[2]
local file = io.open(lua_filename, "w")
file:write(wb_lua)
file:close()

print("Done.")
