-- eLua test 

local cspin, id

if pd.board() == "ATEVK1101" then
  cspin = pio.PA_16
  id = 0
elseif pd.board() == "ATEVK1100" then
  cspin = pio.PA_14
  id = 4
else
  print( "\nError: Unknown board " .. pd.board() .. " !" )
  return
end

function select()
    pio.pin.setlow( cspin ) 
end

function unselect()    
    pio.pin.sethigh( cspin ) 
end


pio.pin.setdir( pio.OUTPUT, cspin )
spi.setup( id, spi.MASTER, 1000000, 0, 0, 8 )

print( "" )

select()
spi.write( id, 0x9F )
print( "JEDEC Manufacturer ID : ", spi.readwrite( id, 0 )[1] )
unselect()

