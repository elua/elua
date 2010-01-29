-- Simple Menu Dispatcher for the LM3S8962 games-only eLua Build

lm3s.disp.init(1000000)

local menuselection = function()
  local kit = require( pd.board() )
  local pressed = {} -- pressed[ button ] is true if the corresponding button was pressed, or nil if not
  local color
  local options = { "Pong", "Tetrives", "Spaceship", "Logo", "Led", "PWMLed", "Snake" }
  local selection = 1

  local function button_clicked( button )
    if kit.btn_pressed( button ) then
    pressed[ button ] = true
    else
    if pressed[ button ] then
      pressed[ button ] = nil
      return true
    end
    end
    return false
  end

  lm3s.disp.print( "Pls select your game:", 0, 5, 15 )
  for id, name in pairs( options ) do
    lm3s.disp.print( name, 30, 10 + ( 11*id ), 10 )
  end

  repeat
    if button_clicked( kit.BTN_UP ) then
      selection = math.max( selection - 1, 1 )
    elseif button_clicked( kit.BTN_DOWN ) then
      selection = math.min( selection + 1, #options )
    end
    for i = 1, #options, 1 do
      if i == selection then
        color = 10
      else
        color = 0
      end
      lm3s.disp.print("*", 22, 10 + ( 11*i ), color )
    end
  until button_clicked( kit.BTN_SELECT )
  return options[ selection ]
end

----- Main ----

local selectedProgram = menuselection()
menuselection = nil
lm3s.disp.clear()
collectgarbage( "collect" )
dofile("/rom/"..selectedProgram..".lua" )
