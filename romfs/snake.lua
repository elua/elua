---------------------------------------------------------------
-- Classic snake game
-- Still testing!
--
--                                      By Ives Negreiros and Téo Benjamin
---------------------------------------------------------------

local xMax = math.floor( 128 / 6 ) - 1
local yMax = math.floor( 96 / 8 ) - 1
local game_map = {}

local Head = {}
local Tail = {}

local highscore = 0
local size = 3
Tail.x = 1
Tail.y = 1
Head.x = Tail.x + ( size - 1 )
Head.y = Tail.y

local Food = {}
Food.x = false
Food.y = false

Head.dx = 1
Head.dy = 0
Tail.dx = Head.dx
Tail.dy = Head.dy
local direction = "right"
local level = 1
local score = 0

lm3s.disp.init( 1000000 )

local kit = require( pd.board() )
local pressed = {}

local function create_food()
--      if not food then
                        Food.x, Food.y = math.random( xMax - 1), math.random( yMax - 1)
                        while game_map[ Food.x ][ Food.y ] do
                          Food.x, Food.y = math.random( xMax - 1 ), math.random( yMax - 1 )
                        end
                        game_map[ Food.x ][ Food.y ] = "food"
                        lm3s.disp.print( "@", Food.x * 6, Food.y * 8, 10 )
--      end
end

local function eat_food()
        lm3s.disp.print( "@", Head.x * 6, Head.y * 8, 0 )
        game_map[ Head.x ][ Head.y ] = nil
        create_food()
        score = score + level
end

local function check_collision()
  if Head.x <= 0 or Head.x >= xMax then
    return true
  elseif Head.y <= 0 or Head.y >= yMax then
    return true
  elseif ( ( game_map[ Head.x ][ Head.y ] ) and ( game_map[ Head.x ][ Head.y ] ~= "food" ) ) then
        return true
  end
  return false
end

local function move()

        if game_map[ Tail.x ][ Tail.y ] == "right" then
                Tail.dx = 1
                Tail.dy = 0
        elseif game_map[ Tail.x ][ Tail.y ] == "left" then
                Tail.dx = -1
                Tail.dy = 0
        elseif game_map[ Tail.x ][ Tail.y ] == "up" then
                Tail.dx = 0
                Tail.dy = -1
        elseif game_map[ Tail.x ][ Tail.y ] == "down" then
                Tail.dx = 0
                Tail.dy = 1
        end
        game_map[ Head.x ][ Head.y ] = direction
        Head.x = Head.x + Head.dx
        Head.y = Head.y + Head.dy

        if game_map[ Head.x ][ Head.y ] == "food" then
                eat_food()
        else
                lm3s.disp.print( "*", Tail.x * 6, Tail.y * 8, 0 )
                game_map[ Tail.x ][ Tail.y ] = nil
                Tail.x = Tail.x + Tail.dx
                Tail.y = Tail.y + Tail.dy
        end

        lm3s.disp.print( "*", Head.x * 6, Head.y * 8, 10 )

end



local function draw_walls()
        for i = 0, xMax*2, 1 do
    lm3s.disp.print( "_", i * 3, yMax * 8 - 6, 11 )
                lm3s.disp.print( "_", i * 3, 0, 11 )
        end
  for i = 0, yMax*2, 1 do
    lm3s.disp.print( "|", xMax * 6, i * 4, 11 )
                lm3s.disp.print( "|", 0, i * 4, 11 )
        end
end

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


function init()
  food = false
  lm3s.disp.clear()
  draw_walls()
  size = 3
  score = 0
  level = 1
  Tail.x = 1
  Tail.y = 1
  Head.x = Tail.x + ( size - 1 )
  Head.y = Tail.y
  Head.dx = 1
  Head.dy = 0
  Tail.dx = Head.dx
  Tail.dy = Head.dy
  direction = "right"

        for i = 0, xMax, 1 do
                game_map[ i ] = {}
        end
  for i = 0, size - 1, 1 do
          game_map[ Tail.x + ( i * Tail.dx ) ][ Tail.y + ( i * Tail.dy ) ] = direction
          lm3s.disp.print( "*", ( Tail.x + ( i * Tail.dx ) ) * 6, ( Tail.y + ( i * Tail.dy ) ) * 8, 10 )
  end
        create_food()
end

--init()
--create_food()

repeat
        init()
  while true do
                local dir = direction
                for i = 1, 1000 - ( 100 * level ), 1 do

                        if kit.btn_pressed( kit.BTN_RIGHT ) and direction ~= "left" then
                                dir = "right"
                                Head.dx = 1
                                Head.dy = 0
                        end
                        if kit.btn_pressed( kit.BTN_LEFT ) and direction ~= "right" then
                                dir = "left"
                                Head.dx = -1
                                Head.dy = 0
                        end
                        if kit.btn_pressed( kit.BTN_UP ) and direction ~= "down" then
                                dir = "up"
                                Head.dx = 0
                                Head.dy = -1
                        end
                        if kit.btn_pressed( kit.BTN_DOWN ) and direction ~= "up" then
                                dir = "down"
                                Head.dx = 0
                                Head.dy = 1
                        end
                        if button_clicked( kit.BTN_SELECT ) and level < 10 then
                                level = level + 1
                        end
                end
                direction = dir
                move()
                if check_collision() then print"colidiu" break end
          --tmr.delay( 0, 400000 )
        --[[
          game_map[ Head.x ][ Head.y ] = nil
        if Head.x < xMax then
                Head.x = Head.x + 1
        end
        game_map[ Head.x ][ Head.y ] = true
    ]]--
                collectgarbage( "collect" )
        end


        if score > highscore then
    highscore = score
  end
    lm3s.disp.clear()                         -- This statements displays the game over screen
    lm3s.disp.print( "Game Over :(", 30, 20, 11 )
    lm3s.disp.print( "Your score was "..tostring( score ), 0, 40, 11 )
    lm3s.disp.print( "Highscore: "..tostring( highscore ), 15, 50, 11 )
    lm3s.disp.print( "SELECT to restart", 6, 70, 11 )
  enough = true                        -- If the player presses select before the time reach 1000000ms, then restart the game
  for i=1, 1000000 do
    if kit.btn_pressed( kit.BTN_SELECT ) then
      enough = false
      break
    end
  end
  lm3s.disp.clear()
until ( enough )
lm3s.disp.off()

