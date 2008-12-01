--[[ To Do
  External Loop to play again, w/ UI msg
  Function lm3s_init will become a separate module, require()d here
--]]

function lm3s_init()
  btn = {}
  btn.UP = pio.PE_0
  btn.DOWN = pio.PE_1
  btn.LEFT = pio.PE_2
  btn.RIGHT = pio.PE_3
  btn.SELECT = pio.PF_1
  btn.LED1 = pio.PF_0
  pio.input( btn.UP, btn.DOWN, btn.LEFT, btn.RIGHT, btn.SELECT )
  pio.pullup( btn.UP, btn.DOWN, btn.LEFT, btn.RIGHT, btn.SELECT )
  pio.output ( btn.LED1 )
  btn.pressed = function( btn )
    return ( pio.get ( btn ) == 0 )
  end
end

function term.keypressed()
  return( uart.recv (0, 0, 0) >= 0 )
end

function drawPaddle( y, color )
  disp.stringdraw("|", 0, y,   color)
  disp.stringdraw("|", 0, y+4, color)
  disp.stringdraw("|", 0, y+8, color)
end

function updateBallPos()
  if( bx >= 121 or bx <= 4) then
    dx = -dx;
  end

  if(( by >= 89 ) or ( by <= 0 )) then
    dy = -dy;
  end
  disp.stringdraw( ball, bx, by, 0 )
  bx, by = ( bx + dx ), ( by + dy );
  disp.stringdraw( ball, bx, by, 15 )
end

function updatePaddlePos()
  if btn.pressed( btn.UP ) then
    if ( py > 0 ) then
      drawPaddle( py, 0 )
      py = py - 1
      drawPaddle( py, 11 )
    end
  elseif btn.pressed( btn.DOWN ) then
    if ( py < 80 ) then
      drawPaddle( py, 0 )
      py = py + 1
      drawPaddle( py, 11 )
    end
  else
    tmr.delay( 0, 400 ) -- Maintain function processing time aprox the same
  end
end

------------ MAIN ------------
lm3s_init()
disp.init(1000000)

term.clrscr()
--term.gotoxy( 5, 1 )
--print( "Welcome to eLua Tenis on a RIT display" )
disp.stringdraw( "eLua Pong", 30, 40, 11 )
tmr.delay ( 0, 2000000 )

highscore = 0

while (true) do
  play = false

  bx, by = 5, 48
  dx, dy = 1, 1
  py = 48

  score = 0
  dscore = 1
  ball = "*"
  time = 10000
  
  change = 0
  
  disp.clear()
  drawPaddle( py, 11 )

  while ( true ) do
    for i = 0, 1 do
      updatePaddlePos()
      tmr.delay ( 0, time )
    end
    updateBallPos()
    if ( bx == 4 ) then
      if (( by+8 < py ) or ( by > py+16 )) then
        break
      else
        score = score + dscore
      end
    end
    
    if change == 0 then
      if btn.pressed( btn.RIGHT ) and time > 0 then
        change = 1
      elseif btn.pressed( btn.LEFT ) and dscore > 1 then
        change = -1
      end
    end
    
    if ( btn.pressed( btn.RIGHT ) ) == false and ( btn.pressed( btn.LEFT ) ) == false then
      if change == 1 then
        time = time - 2000
        dscore = dscore + 1
      elseif change == -1 then
        time = time + 2000
        dscore = dscore - 1
      end      
        change = 0
    end
    
    disp.stringdraw( tostring( dscore ), 118, 0, 6 )
    
  end
  
  if score > highscore then
    highscore = score
  end

  disp.clear()
  disp.stringdraw( "Game Over :(", 30, 20, 11 )
  disp.stringdraw( "Your score was " .. tostring( score ), 15, 40, 11 )
  disp.stringdraw( "High score: " .. tostring( highscore ), 15, 50, 11 )
  disp.stringdraw( "SELECT to restart", 6, 70, 11 )
  for i=0, 500000 do
    if btn.pressed( btn.SELECT ) then
      play = true
      break
    end
  end
  if play == false then
    disp.off()
    break
  end
end
