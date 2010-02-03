-------------------------------------------------------------------------------
--
--          eLua Tetris Game
--
-- LED Lab @ PUC-Rio - 2009
--   Dado Sutter
--   Ives Negreiros
--   Ricardo Rosa
--   Pedro Bittencourt
--   Rafael Barmak
--   Teo Benjamin
--
-- Initial Version by Ives Negreiros, August 2009
--
---------------------------------------------------------------------------------
local Vmax = 22                        -- Number of pieces in vertical +1
local Hmax = 12                        -- Number of pieces horizontally + 2
local score = 0                        -- Player's score
local highscore = 0                    -- Player's highscore
local next_piece = 0                   -- Code for the next piece
local PieceV = 0                       -- Vertical position of the piece
local PieceH = 0                       -- Horizontal position of the piece
local level = 1                        -- Level
local rotate_type = 0                  -- Type of rotation for each piece
local total_lines = 0                  -- Total number of lines made by player
local seed = 0                         -- Variable used to make math.random return a diferent value each time
local game_map = {}                    -- Table for the game map
for i = 1, Vmax, 1 do
  game_map[ i ] = {}
end

-- Initial information
local platform = require( pd.board() )
lm3s.disp.init( 1000000 )

lm3s.disp.print( "eLua Tetrives", 30, 10, 11 )
lm3s.disp.print( "Press SELECT -->", 30, 40, 11 )
while platform.btn_pressed( platform.BTN_SELECT ) == false do
  seed = seed + 1
end

math.randomseed( seed )
lm3s.disp.clear()

function scan_piece( next_piece )      -- This function selec the next piece based on return of math.random function
  if( next_piece == 1 ) then
    next_piece_map = { { 1, 1 }, { 1, 1 } }
    next_rotate_type = 0

  elseif( next_piece == 2 ) then
    next_piece_map = { { 1, 1, 0 }, { 0, 1, 1 }, { 0, 0, 0 } }
    next_rotate_type = 1

  elseif( next_piece == 3 ) then
    next_piece_map = { { 0, 1, 1 }, { 1, 1, 0 }, { 0, 0, 0 } }
    next_rotate_type = 1

  elseif( next_piece == 4 ) then
    next_piece_map = { { 0, 0, 1 }, { 1, 1, 1 }, { 0, 0, 0 } }
    next_rotate_type = 2

  elseif( next_piece == 5 ) then
    next_piece_map = { { 1, 0, 0 }, { 1, 1, 1 }, { 0, 0, 0 } }
    next_rotate_type = 2

  elseif( next_piece == 6 ) then
    next_piece_map = { { 0, 1, 0 }, { 1, 1, 1 }, { 0, 0, 0 } }
    next_rotate_type = 2

  elseif( next_piece == 7 ) then
    next_piece_map = { {0, 0, 0, 0 }, { 1, 1, 1, 1 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } }
    next_rotate_type = 4
  end
end

function draw_walls()                  -- This function draws the walls and the base of game piece screen
  for i = 6, 63, 3 do
    lm3s.disp.print( "|", 3, i, 11 )
  end
  for i = 3, 118, 4 do
	lm3s.disp.print( "-", i, 2, 11 )
	lm3s.disp.print( "-", i, 65, 11 )
  end
end

function sound()                       -- This function beeps
  pwm.start( 1 )
  tmr.delay( 0, 20000 )
  pwm.stop( 1 )
end

function print_data()                  -- This function writes the score and level on screen
  lm3s.disp.print( "Score:"..tostring( score ), 0, 88, 8 )
  lm3s.disp.print( "Level:"..tostring( level ), 0, 80, 8 )
end

function draw_piece()                  -- This function draws the piece on the screen
  for i in ipairs( piece_map ) do
    for j in ipairs( piece_map[ i ] ) do
      if( piece_map[ i ][ j ] == 1 ) then
        lm3s.disp.print( "*", ( PieceV + i - 1 ) * 6, ( PieceH + j - 1 ) * 6, 11 )
      end
    end
  end
end

function erase_piece()                 -- This function erases the piece on the screen
  for i in ipairs( piece_map ) do
    for j in ipairs( piece_map[ i ] ) do
      if( piece_map[ i ][ j ] == 1 ) then
        lm3s.disp.print( "*", ( PieceV + i - 1 ) * 6, ( PieceH + j - 1 ) * 6, 0 )
      end
    end
  end
end

function move_down()                   -- This function moves the piece down if there is no obstacle in the way, else create new piece
  free = 0                              -- It also test the lines (see function test_line for further explanation)
  for i in ipairs( piece_map ) do
    for j in ipairs( piece_map[ i ] ) do
      if( piece_map[ i ][ j ] == 1 ) then
        if( game_map[ PieceV + i - 1 ][ PieceH + j ] == 0 ) then
          free = free + 1
        end
      end
    end
  end
  if ( free == 4 ) then
    erase_piece()
    PieceV = PieceV - 1
    draw_piece()
  else
    for i in ipairs( piece_map ) do
      for j in ipairs( piece_map[ i ] ) do
        if( piece_map[ i ][ j ] == 1 ) then
          game_map[ PieceV + i ][ PieceH + j ] = 1
        end
      end
    end
    test_line()
    create_new_piece()
  end
end

function move_left()                   -- This function moves the piece left if there is no obstacle in the way
  free = 0
  for i in ipairs( piece_map ) do
    for j in ipairs( piece_map[ i ] ) do
      if( piece_map[ i ][ j ] == 1 ) then
        if( game_map[ PieceV + i ][ PieceH + j - 1 ] == 0 ) then
          free = free + 1
        end
      end
    end
  end
  if ( free == 4 ) then
    erase_piece()
    PieceH = PieceH - 1
    draw_piece()
  end
end

function move_right()                  -- This function moves the piece right if there is no obstacle in the way
  free = 0
  for i in ipairs( piece_map ) do
    for j in ipairs( piece_map[ i ] ) do
      if( piece_map[ i ][ j ] == 1 ) then
        if( game_map[ PieceV + i ][ PieceH + j + 1 ] == 0 ) then
          free = free + 1
        end
      end
    end
  end
  if ( free == 4 ) then
    erase_piece()
    PieceH = PieceH + 1
    draw_piece()
  end
end

function rotate()                      -- This function rotate the pieces
  piecerot = {}
  for i = 1, 4, 1 do
    piecerot[ i ] = {}
  end
  free = 0
  erase_piece()
  if ( rotate_type == 1 or rotate_type == 2 ) then
    for i in ipairs( piece_map ) do              -- this loop test if each part of piece can rotate
      for j in ipairs( piece_map[ i ] ) do
        if( piece_map[ i ][ j ] == 1 ) then
          if( game_map[ PieceV + j ][ 4 - i + PieceH ] == 0 ) then
            free = free + 1
          end
        end
      end
    end
    if( free == 4 ) then               -- If all the parts of piece can rotate, then 'free' will be equals to four
      if( rotate_type == 2 ) then
        rotate_type = 1
      end
      for i in ipairs( piece_map ) do            -- This loop rotates the piece
        for j in ipairs( piece_map[ i ] ) do
          piecerot[ i ][ j ] = piece_map[ j ][ 4 - i ]     -- This is the equation of the rotation function for this type of rotation
        end
      end
      piece_map = piecerot
    end

  elseif ( rotate_type == 3 ) then      -- This part works like the upper part, but for another type of rotation
    for i in ipairs( piece_map ) do
      for j in ipairs( piece_map[ i ] ) do
        if( piece_map[ i ][ j ] == 1 ) then
          if( game_map[ 4 - j + PieceV ][ PieceH + i ] == 0 ) then
            free = free + 1
          end
        end
      end
    end
    if( free == 4 ) then
      rotate_type =  2
      for i in ipairs( piece_map ) do
        for j in ipairs( piece_map[ i ] ) do
          piecerot[ i ][ j ] = piece_map[ 4 - j ][ i ]
        end
      end
      piece_map = piecerot
    end

  elseif ( rotate_type == 4 ) then     -- This part works like the upper part, but for another type of rotation
    for i in ipairs( piece_map ) do
      for j in ipairs( piece_map[ i ] ) do
        if( piece_map[ i ][ j ] == 1 ) then
          if( game_map[ PieceV + j ][ PieceH + i ] == 0 ) then
            free = free + 1
          end
        end
      end
    end
    if(free == 4 ) then
      for i in ipairs( piece_map ) do
        for j in ipairs( piece_map [ i ] ) do
          piecerot[ i ][ j ] = piece_map[ j ][ i ]
        end
      end
      piece_map = piecerot
    end
  end
  draw_piece()
  sound()
end

function remove_line( line )
  for i = line, Vmax - 2, 1 do
    for j = 2, Hmax - 1, 1 do
      lm3s.disp.print( "*", ( i - 1 ) * 6, ( j - 1 ) * 6, 0 )
      game_map[ i ][ j ] = game_map[ i + 1 ][ j ]
      if( game_map[ i ][ j ] == 1 ) then
        lm3s.disp.print( "*", ( i - 1 ) * 6, ( j - 1 ) * 6, 11 )
      end
    end
  end
end

function test_line()                   -- This function tests the lines, if there is a full line, then this
  lines = 0                            -- function removes this line and move down everything that is upper to it
  i = 2
  while ( i<Vmax ) do
    j = 2
    while ( j<Hmax ) do
      if( game_map[i][j] == 0 ) then
        break
      elseif( j == Hmax - 1 ) then
        remove_line( i )
        lines = lines + 1
        i = i - 1
        break
      end
      j = j + 1
    end
    i = i + 1
  end
  total_lines = total_lines + lines
  score = score + 100 * level * lines * lines    -- This is the euqation for the score value
  if( total_lines >= 8 and level < 4 ) then
    level = level + 1
    total_lines = 0
  end
end

function create_new_piece()            -- This Function creates a new piece
  piece_map = next_piece_map
  rotate_type = next_rotate_type
  PieceV = 18
  PieceH = 4
  next_piece = math.random( 7 )
  scan_piece ( next_piece )
  for i = 1, 2, 1 do
    for j = 1, 4, 1 do
      lm3s.disp.print( "*", 94 + ( j * 6 ), 78 + ( i * 6 ), 0 )
    end
  end
  for i in ipairs( next_piece_map ) do
    if( i == 3 ) then
      break
    end
    for j in ipairs( next_piece_map[ i ] ) do
      if( next_piece_map[ i ][ j ] == 1 ) then
        lm3s.disp.print( "*", 94 + ( j * 6 ), 78 + ( ( 3 - i ) * 6 ), 11 )
      end
    end
  end
  draw_piece()
end

---------------------------------------------------------------------------------
--																			   --
--				                 MAIN LOOP							           --
--																			   --
---------------------------------------------------------------------------------
repeat

  for i in ipairs( game_map ) do       -- This loop create the border of game's map
    for j = 1, Hmax, 1 do
      if( j == 1 or j == Hmax or i == 1 or i == Vmax ) then
        game_map[ i ][ j ] = 1
      else
        game_map[ i ][ j ] = 0
      end
    end
  end
  level = 1                            -- This statements sets the level and score from the beginning
  score = 0
  pwm.setclock( 1, 25000000 )          -- This statements sets the PWM for the sound function
  pwm.setup( 1, 1000, 70 )
  draw_walls()
  next_piece = math.random(7)
  scan_piece ( next_piece )
  create_new_piece()
  collectgarbage( "collect" )
  while true do                        -- This loop refreshes the data and responds the player's input
    print_data ()
    Tmax = 11 - 2 * level              -- This statement raises the speed based on the level
    for i = 1, Tmax, 1 do
      if platform.btn_pressed( platform.BTN_UP ) then
        move_left()
      end
      if platform.btn_pressed( platform.BTN_DOWN ) then
        move_right()
      end
      if platform.btn_pressed( platform.BTN_RIGHT ) then
        rotate()
      end
      if platform.btn_pressed( platform.BTN_LEFT ) then   -- If the player presses "down", the piece drops instantly and a point is added to score
        score = score + 1
        tmr.delay( 1, 30000 )
        break
      end
      tmr.delay( 1, 70000 )
    end
    move_down()
    if( game_map[ PieceV + 2 ][ PieceH + 2 ] == 1 ) then   -- If this condition is true then game over
      break
    end
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
    if platform.btn_pressed( platform.BTN_SELECT ) then
      enough = false
      break
    end
  end
  lm3s.disp.clear()
until ( enough )
lm3s.disp.off()
