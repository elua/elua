local f = io.open( "dis.out", "rb" )
local sf = string.format
local instr_start = 22
local max_prefix_block = 15

lines = {}
for l in f:lines() do
  lines[ #lines + 1 ] = l
end

f:close()

local nlines = #lines
print( sf( "Read %d lines", nlines ) )

-- Look for all branch instructions to __cyg_profile_func_enter
local ncalls = 0
local opcodes = {}
local first_opcodes = {}
for i = 1, nlines do
  local l = lines[ i ]
  local instr = l:sub( instr_start )
  if instr:match( "__cyg_profile_func_enter" ) then
    -- Found call instruction, look at the instructions above
    local last_line, first_line = i - 1, i - 1
    while true and first_line > 0 and last_line - first_line < max_prefix_block do
      if lines[ first_line ]:match( "^%x%x%x%x%x%x%x%x <" ) then break end
      first_line = first_line - 1
    end
    first_line = first_line + 1
    if first_line <= 0 then
      error( sf( "unable to find first_line for last_line = %d!", last_line ) )
    end
    if last_line - first_line >= max_prefix_block then
      print( sf( "sequence too long for last_line = %d!", last_line ) )
    end
    -- Look between [first_line, last_line] and check all instruction there
    for n = first_line, last_line do
       l = lines[ n ]
       local opcode = l:sub( 11, 21 ):gsub( "%s+", "" )
       instr = l:sub( 22 ):match( "([^%s]+)" )
       opcodes[ opcode ] = instr
       if n == first_line then
        if instr ~= "stmdb" and instr ~= "push" then
          print( instr, n )
        end
        first_opcodes[ opcode ] = true
       end
    end
    ncalls = ncalls + 1
  end
end

print( sf( "Found %d calls to __cyg_profile_func_enter", ncalls ) )
--for o, i in pairs( opcodes ) do print( o ) end
for k, v in pairs( first_opcodes ) do print( k ) end

