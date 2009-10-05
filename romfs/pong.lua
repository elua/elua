-------------------------------------------------------------------------------
--
--		eLua Pong Game
--
-- LED Lab @ PUC-Rio - 2009
--   Dado Sutter
--   Ives Negreiros
--   Ricardo Rosa
--   Pedro Bittencourt
--   Teo Benjamin
--

-- Initial Version by Dado Sutter, Fev 2009
--    This had only the ball bouncing on walls, paddle and paddle movement
--
-- Greatly enhanced by Teo Benjamin in Aug/Sep 2009, adding:
--    Score, resizeable paddles, levels/speeds, items
--
--
-------------------------------------------------------------------------------

local canvas = {}
-- canvas.x                     -- Horizontal display size
-- canvas.y                     -- Vertical display size

local paddle = {}
-- paddle.size                  -- Actual Paddle size = ( 6 * ( paddle.size + 1 ) ) + 2
-- paddle.max_size              -- Max paddle.size value -> Constant
-- paddle.min_size              -- Min paddle.size value -> Constant
-- paddle.y                            -- Paddle's Y position ( X position not needed, always 0 )

local ball = {}
-- ball.x                       -- Ball's X position
-- ball.y                       -- Ball's Y position -> starts at a random position
-- ball.dx                      -- Ball's X movement ( 1 = moving right; -1 = moving left )
-- ball.dy                      -- Ball's Y movement ( 1 = moving down; -1 = moving up )
-- ball.char                    -- The char that is printed for the ball -> Constant

local item = {}
-- item.x                       -- Item's X position
-- item.y                       -- Item's Y position ( fix for each item )
-- item.char                    -- This is the char that represents the item ( if false, there is no item )
-- item.all_chars                     -- A table that contains all the possibles item chars. Initialized by upload_items() function

-- Define all constants
local tmr_id = 1
paddle.max_size = 4
paddle.min_size = 0
ball.char = "*"
local delay_incr = 2000


-- Define all "global" variables as program local ones.
-- The values are initialized at the main loop.

local score                    -- Player's score
local dscore                   -- How many points for each paddle hit
local delay_time               -- This value is used for the main delay, to make the game speed faster or slower
local paddle_hits              -- Counts the number of hits on paddle
local highscore                -- Current Highscore

item.all_chars = {}
local pressed = {}             -- pressed[ button ] is true if the corresponding button was pressed, or nil if not

local kit = require( pd.board() )  -- This variable is used as a pin assignments for the specific board

local itemFunction = {
["L"] = function ()
    draw_paddle( paddle.y, 0, 0 )
    if paddle.size < paddle.max_size then
      paddle.size = paddle.size + 1
    end
    draw_paddle( paddle.y, 11, 0 )
  end,
["S"] = function ()
    draw_paddle( paddle.y, 0, 0 )
    if paddle.size > paddle.min_size then
      paddle.size = paddle.size - 1
    end
    draw_paddle( paddle.y, 11, 0 )
  end,

["?"] = function ()
    item.char = item.all_chars[ math.random( #item.all_chars ) ]
    use_item()
  end,

["*"] = function ()
  end,

["P"] = function ()
    score = score + dscore
  end,

["D"] = function ()
    score = score * 2
  end,

["Z"] = function ()
    lm3s.disp.print( tostring( score ), 111, 89, 0 )
    score = 0
  end,

["T"] = function ()
    lm3s.disp.print( ball.char, ball.x, ball.y, 0 )
    ball.y = math.random( 82 )
    lm3s.disp.print( ball.char, ball.x, ball.y, 15 )
  end,
["F"] = function()
    if delay_time >= 1000 then
      delay_time = delay_time - 1000
    end
  end,
}

-- Updates Y paddle position and draw it using the draw_paddle( ... ) function
function update_paddle_pos()
  if kit.btn_pressed( kit.BTN_UP ) then
    if ( paddle.y > 0 ) then
      paddle.y = paddle.y - 1
      draw_paddle( paddle.y, 11, -1 )
    else
      tmr.delay( 1, 1700 )
    end
  elseif kit.btn_pressed( kit.BTN_DOWN ) then
    if ( paddle.y + ( paddle.size*6 ) + 1 < 90 ) then
      paddle.y = paddle.y + 1
      draw_paddle( paddle.y, 11, 1 )
    else
      tmr.delay( 1, 1600 )
    end
  else
    draw_paddle( paddle.y, 11, 0 )
    tmr.delay( 1, 300 ) -- Maintain function processing time aprox the same
  end
end

-- Draw the paddle in the display. This function is used by update_paddle_pos() function
function draw_paddle( y, color, movement )
  if ( movement == 0 ) then
    for i = 0, paddle.size, 1 do
      lm3s.disp.print( "|", 0, y + ( i * 6 ),  color )
    end
  elseif ( movement > 0 ) then      -- Paddle moving Down
    if y < 8 then
      lm3s.disp.print( "|", 0, 0,  0 )
    else
      lm3s.disp.print( "|", 0, y - 8 , 0 )
    end
    for i = 0, paddle.size, 1 do
      lm3s.disp.print( "|", 0, y + ( i * 6 ),  color )
    end
  elseif ( movement < 0 ) then    -- Paddle moving Up
    lm3s.disp.print( "|", 0, y + ( ( paddle.size + 1 ) * 6 ) + 2 , 0 )
    for i = 0, paddle.size, 1 do
      lm3s.disp.print( "|", 0, y + ( i * 6 ),  color )
    end
  end
end

-- Updates the ( X, Y ) ball position and prints the corresponding char
function update_ball_pos()
  if( ( ball.x + 5 >= canvas.x ) or ( ball.x <= 4 ) ) then
    ball.dx = -ball.dx;
    if ball.dx == -1 and item.char == false then
      createItem()
    end
  end
  if( ( ball.y >= 90 - ball.dy ) or ( ball.y <= 1 - ball.dy ) ) then
    ball.dy = -ball.dy;
  end
  lm3s.disp.print( ball.char, ball.x, ball.y, 0 )
  ball.x, ball.y = ( ball.x + ball.dx ), ( ball.y + ball.dy );
  lm3s.disp.print( ball.char, ball.x, ball.y, 15 )
end


-- Draw the top wall and erase the last one. Used to move it
function draw_wall( x )
  for i = 0, canvas.y, 7 do                    -- Erase the wall
    lm3s.disp.print( "|", canvas.x + 1, i, 0 )
  end
  canvas.x = x
  for i = 0, canvas.y, 7 do                    -- Draw a new wall
    lm3s.disp.print( "|", canvas.x + 1, i, 6 )
  end
end

-- Item Functions
function createItem()
  item.char = item.all_chars[ math.random( #item.all_chars ) ]
  item.x = canvas.x - 10
  item.y = ball.y
end

-- Upload the itens from table itemFunction into the table item.all_chars
-- Must be used to initialize the items
function upload_items()
  for k,v in pairs( itemFunction ) do
    table.insert( item.all_chars, k )
  end
end

-- Updates the item X position
function update_item_pos()
  if item.char then
    if ( item.x <= 4 ) then
      if ( ( item.y + 8 < paddle.y ) or ( item.y > paddle.y + ( paddle.size * 6 ) + 8 ) ) == false then
        use_item()
      end
      lm3s.disp.print( item.char, item.x, item.y, 0 )
      item.char = false
      return
    end
    lm3s.disp.print( item.char, item.x, item.y, 0 )
    item.x = item.x - 2
    lm3s.disp.print( item.char, item.x, item.y, 10 )
  end
end

-- Uses the item's function
function use_item()
  itemFunction[ item.char ]()
end

-- Checks if a button was clicked ( pressed and released )
-- Returns true or false
function button_clicked( button )
  if kit.btn_pressed( button ) then
    pressed[ button ] = true
  else
    if pressed[ button ] then
      pressed[ button ] = nil
      return true
    end
    pressed[ button ] = nil
  end
  return false
end

------------ MAIN ------------
upload_items()
lm3s.disp.init( 1000000 )

tmr.start( tmr_id )
--menu()
math.randomseed( tmr.read( tmr_id ) )    -- If you use the menu function, the time will be used as a seed to the random function
--tmr.stop( tmr_id )

collectgarbage( "collect" )





---------------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------------
--                                                                                                     --
--                                        GAME START                                                   --
--                                                                                                     --
---------------------------------------------------------------------------------------------------------
---------------------------------------------------------------------------------------------------------

repeat
  canvas.x = 124
  canvas.y = 97
  ball.x = 5
  ball.y = math.random( ( canvas.y - 8 ) / 2 ) + ( canvas.y / 4 )
  ball.dx = 1
  ball.dy = 1
  paddle.y = ball.y - 4
  item.x = 0
  item.y = 0
  score = 0
  dscore = 1
  item.char = false
  paddle.size = 2
  delay_time = 10000
  paddle_hits = 0

  lm3s.disp.clear()

  draw_wall( canvas.x )
  draw_paddle( paddle.y, 11, 0 )

  while ( true ) do
    for i = 0, 1 do
      update_paddle_pos()
    end
    tmr.delay ( 0, delay_time )
    update_ball_pos()
    update_item_pos()
    if ( ball.x <= 4 ) then
      if ( ( ball.y + 8 < paddle.y ) or ( ball.y > paddle.y + ( paddle.size * 6 ) + 8 ) ) then  -- If this is true, you lose
        break
    else                                                                     -- Else, you score
        score = score + dscore
        paddle_hits = paddle_hits + 1
      end
    end

    if button_clicked( kit.BTN_RIGHT ) and delay_time > 0 then   -- If the right button is clicked, increase the level
      delay_time = delay_time - delay_incr
      dscore = dscore + 1
    end
    if button_clicked( kit.BTN_LEFT ) and dscore > 0 then    -- If the left button is clicked, decrease the level
      delay_time = delay_time + delay_incr
      dscore = dscore - 1
    end

    if ( paddle_hits == 5 ) and canvas.x > 80 then  -- After 5 hits in a row, move the wall until canvas.x = 80
      paddle_hits = 0
      draw_wall( canvas.x - 5 )
    end
    lm3s.disp.print( tostring( dscore ), 118, 0, 6 )
    lm3s.disp.print( tostring( score ), 111, 89, 6 )
    collectgarbage( "collect" )
  end
-------------------------------------------
-- Game Over
-------------------------------------------
  if score >= ( highscore or 0 ) then
    highscore = score
  end
  lm3s.disp.clear()
  lm3s.disp.print( "Game Over :(", 30, 20, 11 )
  lm3s.disp.print( "Your score was "..tostring( score ), 15, 40, 11 )
  lm3s.disp.print( "Highscore: "..tostring( highscore ), 15, 50, 11 )
  lm3s.disp.print( "SELECT to restart", 6, 70, 11 )
  enough = true
  for i=1, 100000 do
    if kit.btn_pressed( kit.BTN_SELECT ) then
      enough = false
      break
    end
  end
until ( enough )

lm3s.disp.off()
