-- [TODO] remove this!

local function int_handler( type )
  print "Fired!"
end

cpu.set_int_handler( int_handler )
cpu.sei( 12 )

print( "I'm running on platform " .. pd.platform() )
print( "The CPU is a " .. pd.cpu() )
print( "The board name is " .. pd.board() )

while uart.getchar( 0, 0 ) == "" do end
cpu.cli( 12 )


