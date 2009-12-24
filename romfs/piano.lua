-- "Piano" in eLua, showing yet another use for the PWM module :)

local pwmid, tmrid

if not math then
  print "This example required floating point Lua (integer Lua not supported)" 
  return
end
  
if pd.board() == "EK-LM3S8962" or pd.board() == "EK-LM3S6965" or pd.board() == "EAGLE-100" then
  pwmid, tmrid = 1, 1
elseif pd.board() == "SAM7-EX256" then
  pwmid, tmrid = 0, 1
  tmr.setclock( 1, 1000000 )
elseif pd.board() == "STR-E912" then
  local g = str9.pio
  g.setpin( pio.P4_6, g.OUTPUT, g.OUTPUT_PUSHPULL, false, g.ALT_OUTPUT2 )
  pwmid, tmrid = 3, 1
else
  print( pd.board() .. " not supported with this example" )
  return
end

local oct, pause = 4, 0

-- Mapping between keys and note offsets
local offsets = { a = 0, w = 1, s = 2, d = 3, r = 4, f = 5, t = 6, g = 7, h = 8,
  u = 9, j = 10, i = 11, k = 12, o = 13, l = 14, [";"] = 15, ["["] = 16, ["'"] = 17,
  ["]"] = 18, ["\\"] = 19 }
  
-- Write the curent octave
function show_octave()
  term.print( 2, 4, "Oct: " .. tostring( oct ) .. "(+/-)" )
  term.moveto( 2, 19 )  
end

-- Write the current pause between notes
function show_pause()
  term.moveto( 2, 5 )
  term.clreol()
  term.print( 2, 5, "Pause between notes: " .. tostring( pause ) .. "ms (</>)" )
  term.moveto( 2, 19 )  
end

-- Show the main interface
function show_all()
  term.print( 2, 2, "eLua piano demo" )  
  show_octave()
  show_pause()
  term.print( 4,  7, " w   r  t   u  i  o   [  ]  " )
  term.print( 4,  8, " |   |  |   |  |  |   |  |  " )
  term.print( 4,  9, " A#  C# D#  F# G# A#  C# D# " )
  term.print( 4, 10, "A  BC  D  EF  G  A  BC  D  E" )
  term.print( 4, 11, "|  ||  |  ||  |  |  ||  |  |" )
  term.print( 4, 12, "a  sd  f  gh  j  k  l;  '  \\" )  
  term.print( 2, 14, "Use above keys to play notes." )
  term.print( 2, 15, "+/- to change octave." )
  term.print( 2, 16, "</> to change pause between notes." )
  term.print( 2, 17, "Space to stop playing." ) 
  term.print( 2, 18, "ESC to exit." ) 
  term.moveto( 2, 19 )
end

-- Conversion of note to frequency
function note_to_freq( index )
  return 55 * 2 ^ ( ( index + ( oct - 1 ) * 12 ) / 12 )
end

-- Entry point
term.clrscr()
pwm.setclock( pwmid, 1000000 )
show_all()
while true do
  local key = term.getchar()
  if key == term.KC_ESC then break end
  local res, strkey = pcall( string.char, key )
  if res then     
    if offsets[ strkey ] ~= nil then
      local freq = note_to_freq( offsets[ strkey ] )
      pwm.stop( pwmid )
      if pause > 0 then
        tmr.delay( tmrid, pause * 1000 )
      end
      pwm.setup( pwmid, freq, 50 )
      pwm.start( pwmid )
    elseif strkey == ">" then
      pause = pause < 1000 and pause + 10 or pause
      show_pause()
    elseif strkey == "<" then
      pause = pause > 0 and pause - 10 or pause
      show_pause() 
    elseif strkey == "+" then
      oct = oct < 7 and oct + 1 or oct
      show_octave()
    elseif strkey == "-" then
      oct = oct > 1 and oct - 1 or oct
      show_octave()
    elseif strkey == " " then
      pwm.stop( pwmid )
    end
  end  
end

pwm.stop( pwmid )
term.clrscr()
term.moveto( 1, 1 )
