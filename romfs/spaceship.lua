-------------------------------------------------------------------------------
--
--		eLua SpaceShip Game
--
-- LED Lab @ PUC-Rio - 2009
--   Dado Sutter
--   Ives Negreiros
--   Ricardo Rosa
--   Pedro Bittencourt
--   Teo Benjamin
--
--
-- Initial Version by Téo Benjamin, Aug 2009
--
-------------------------------------------------------------------------------

local xcanvas = 124						-- Real screen X size
local xcanvas = xcanvas - 12			-- Canvas size with wall
local ycanvas = 96						-- Screen Y size
local pressed = {}						-- This table is used to help the buttonClicked function. If the btn if pressed,
										--	pressed[ btn ] = true, when it is released, pressed[ btn ] = nil
local ShipChar = ">"					-- This is the char that is printed to represent the SpaceShip
local shotChar = "-"					-- This char is printed to represent the shots
local enemyChar = "@"					-- This char is used to represent the enemies
local sy = 16							-- Ship's Y position
local sx = 0							-- Ship's X position
local bullets = 50						-- Number of bullets left
local score = 0							-- Player's score
local numOfEnemies = 5					-- Number of simultaneous enemies
local won = false						-- If the player won the game, this value is true. If not, it's false.
-- Initialization
local kit = require( pd.board() )


local canvasMap = {}	-- canvasMap[ line ][ #shot ]
-- canvasMap[ i ][ j ]: i represents the y position and j is a numeric index for each shot in that line. The value is the x position.
-- canvasMap[ i ].e represents the x position of the enemy in that line, if any.
-- Enemies can only appear in lines 5, 15, 25, 35, ..., 75 and 85
for i = 1, ycanvas, 1 do
	canvasMap[ i ] = {}
end
for i = 5, 85, 10 do
	canvasMap[ i ].e = false
end

function drawWall( x, y )
	for i = 0, y, 7 do
  	lm3s.disp.print( "|", xcanvas + 1, i, 0 )
  end
  xcanvas = x
  for i = 0, y, 7 do
  	lm3s.disp.print( "|", xcanvas + 1, i, 6 )
  end

end

function drawShip( x, y, color, movement )

	if ( movement == 0 ) then
  		lm3s.disp.print( ShipChar, x, y,  color )
	elseif ( movement > 0 ) then -- Moving Down
		if y < 8 then
			lm3s.disp.print( ShipChar, x, 0,  0 )
		else
			lm3s.disp.print( ShipChar, x, y - 8 , 0 )
		end
  	lm3s.disp.print( ShipChar, x, y,  color )
  elseif ( movement < 0 ) then -- Moving Up
		lm3s.disp.print( ShipChar, x, y + 8, 0 )
  	lm3s.disp.print( ShipChar, x, y,  color )
  end
end


function updateShipPos()
  if kit.btn_pressed( kit.BTN_UP ) then
    if ( sy > 1 ) then
      sy = sy - 1
      drawShip( sx, sy, 11, -1 )
    else
    	tmr.delay( 1, 1700 )
    end
  elseif kit.btn_pressed( kit.BTN_DOWN ) then
    if ( sy + 7 < ycanvas ) then
      sy = sy + 1
      drawShip( sx, sy, 11, 1 )
    else
    	tmr.delay( 1, 1600 )
    end
  else
  	drawShip( sx, sy, 11, 0 )
    tmr.delay( 1, 300 ) -- Maintain function processing time aprox the same
  end
end



function updateShots()
	for i in ipairs( canvasMap ) do
		for j in ipairs( canvasMap[ i ] ) do
			if canvasMap[ i ][ j ] then
				lm3s.disp.print( shotChar, canvasMap[ i ][ j ], i, 0 )
				canvasMap[ i ][ j ] = canvasMap[ i ][ j ] + 2
				lm3s.disp.print( shotChar, canvasMap[ i ][ j ], i, 13 )
				if canvasMap[ i ][ j ] + 4 >= xcanvas then
					lm3s.disp.print( shotChar, canvasMap[ i ][ j ], i, 0 )
					table.remove( canvasMap[ i ], j )
					break
				end
				local en = math.floor( i / 10 ) * 10 + 5
				if canvasMap[ en ].e then
					if ( canvasMap[ i ][ j ] <= canvasMap[ en ].e ) and ( canvasMap[ i ][ j ] + 4 >= canvasMap[ en ].e ) then
						destroyEnemy( i, j, en )
						createEnemy()
					end
				end
			else
				tmr.delay( 1, 1200 )
			end
		end
	end
end

function shot()
	if bullets > 0 then
		table.insert( canvasMap[ sy ], sx + 6 )
		bullets = bullets - 1
	end
	sound()
end

function buttonClicked( button )
	if kit.btn_pressed( button ) then
		pressed[ button ] = true
	else
		if pressed[ button ] then
			pressed[ button ] = false
			return true
		end
		pressed[ button ] = false
	end
	return false
end

function printBulletsNum()
	lm3s.disp.print( string.format( "%2d", bullets ), xcanvas + 4, 2, 6 )
end

function printScore()
	lm3s.disp.print( string.format( "%2d", score ), xcanvas + 4, ycanvas - 10, 6 )
end

function sound()
	pwm.start( 1 )
	tmr.delay( 0, 20000 )
	pwm.stop( 1 )
end


function updateEnemiesPos()
	for i = 5, 85, 10 do
		if canvasMap[ i ].e then
			lm3s.disp.print( enemyChar, canvasMap[ i ].e, i, 0 )
			canvasMap[ i ].e = canvasMap[ i ].e - 1
			lm3s.disp.print( enemyChar, canvasMap[ i ].e, i, 11 )
			if canvasMap[ i ].e <= 0 then
				lm3s.disp.print( enemyChar, canvasMap[ i ].e, i, 0 )
				canvasMap[ i ].e = nil
				createEnemy()
			end
		end
	end
end

function createEnemy()
	while true do
		local en = ( math.random( 0, 8 ) )*10 + 5
		if not canvasMap[ en ].e then
			canvasMap[ en ].e = xcanvas - 5
			break
		end
	end
end

function addEnemy()
	if numOfEnemies < 9 then
		numOfEnemies = numOfEnemies + 1
		createEnemy()
	end
end
function destroyEnemy( i, j, en )
	lm3s.disp.print( shotChar, canvasMap[ i ][ j ], i, 0 )
	table.remove( canvasMap[ i ], j )
	lm3s.disp.print( enemyChar, canvasMap[ en ].e, en, 0 )
	canvasMap[ en ].e = nil
	score = score + 1
end

function destroyAll()
	for i in ipairs( canvasMap ) do
		for j in ipairs( canvasMap[ i ] ) do
			lm3s.disp.print( shotChar, canvasMap[ i ][ j ], i, 0 )
			table.remove( canvasMap[ i ], j )
		end
		local en = math.floor( i / 10 ) * 10 + 5
		if canvasMap[ en ].e then
			lm3s.disp.print( enemyChar, canvasMap[ en ].e, en, 0 )
			canvasMap[ en ].e = nil
		end
	end

end

for i = 1, numOfEnemies, 1 do
	createEnemy()
end


lm3s.disp.init( 1000000 )

-- Initial information
lm3s.disp.print( "eLua SpaceShip", 10, 10, 11 )
lm3s.disp.print( "Press SELECT -->", 10, 40, 11 )
local seed = 0
while not buttonClicked( kit.BTN_SELECT ) do
	seed = seed + 1
end
math.randomseed( seed )
lm3s.disp.clear()

-------------------------------------------------------------------------------
--
--				MAIN LOOP
--
-------------------------------------------------------------------------------
pwm.setclock( 1, 25000000 )
pwm.setup( 1, 1000, 70 )

drawWall( xcanvas, ycanvas )

while true do
	updateEnemiesPos()
	for i = 1, 3, 1 do
		updateShipPos()
		updateShots()
	end
	printBulletsNum()
	printScore()
	if buttonClicked( kit.BTN_SELECT ) then shot() end
	if buttonClicked( kit.BTN_RIGHT ) then
		destroyAll()
		for i = 1, numOfEnemies, 1 do
			createEnemy()
		end
	end
	if score >= 50 then
	  won = true
	  break
	end
	if bullets <= 0 then
	  won = false
	  break
	end
	tmr.delay(1, 12000)
	collectgarbage("collect")
end
if won then
  lm3s.disp.clear()
  lm3s.disp.print( "You won!", 50, 30, 11 )
  lm3s.disp.print( "Congratulations!", 70, 20, 11 )
else
  lm3s.disp.clear()
  lm3s.disp.print( "Game Over! :(", 60, 20, 11 )
end
