-- [TODO] remove this!

local function int_handler( type )
  print "Fired!"
end

cpu.set_int_handler( int_handler )

print( "I'm running on platform " .. pd.platform() )
print( "The CPU is a " .. pd.cpu() )
print( "The board name is " .. pd.board() )

while true do end

