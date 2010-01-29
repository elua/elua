-- Hangman in eLua using the 'term' module
-- Inspired by the original 'hangman' from the bsdgames package

-- we need a random function
-- using math.random for now, which implies target=lua, not lualong
if not math then
  print "\nError: Hangman needs the math module (disabled when target=lualong) !"
  return
end

local h, w = term.getlines(), term.getcols()
local tries = 0

-- "Database" with our words
local words = { "simple", "hangman", "guess", "elua", "inane", "serial", 
  "stupenduous", "software" }

-- Build our hanging site :)
function hang()
  if tries == 0 then
    -- Build the basic structure
    term.print( 5, 1, string.rep( '_', 6 ) )
    term.print( 5, 2, '|    |')
    for i = 3, 6 do
      term.print( 5, i, '|' )
    end
    term.print( 3, 7, '__|_____')
    term.print( 3, 8, '|      |___')
    term.print( 3, 9, '|__________|') 
    
  elseif tries == 1 then
    -- Draw the head
    term.print( 10, 3, "O" )
    
  elseif tries == 2 or tries == 3 then
    -- First or second part of body
    term.print( 10, tries + 2, "|" )
    
  elseif tries == 4 or tries == 5 then
    -- First leg / first hand
    term.print( 9, tries == 4 and 6 or 4, "/" )
  
  elseif tries == 6 or tries == 7 then
    -- Second hand / second leg
    term.print( 11, tries == 7 and 6 or 4, "\\" )
  end  
end

local total, guessed = 0, 0

-- Show the game statistics
function stats()
  term.print( w - 20, 5, "Total words: ", tostring( total ) )
  term.print( w - 20, 6, "Guessed words: ", tostring( guessed ) )
end

while true do
  term.clrscr()
  term.print( 3, 12, "eLua hangman" )
  term.print( 3, 13, "ESC to exit" )
  stats()
  
  -- Draw the hanging site
  tries = 0
  hang()
    
  -- Then write the "Guess" line
  term.print( 2, h - 3, "Word: " )
  local lword = words[ math.random( #words ) ]:lower()
  term.print( string.rep( "-", #lword ) )
  term.print( 2, h - 2, "Guess: " )
  
  local nguess = 0
  local tried = {}
  local key
  while tries < 7 and nguess < #lword do     
    key = term.getchar()
    if key == term.KC_ESC then break end
    if key > 0 and key < 255 then
      key = string.char( key ):lower()
      term.moveto( 2, h - 1 )
      term.clreol()       
      if not key:find( '%l' ) then
        term.print( "Invalid character" )
      else
        key = key:byte()
        if tried[ key ] ~= nil then
          term.print( "Already tried this key" )
        else
          tried[ key ] = true
          local i
          local ok = false
          for i = 1, #lword do
            if key == lword:byte( i ) then
              ok = true
              term.print( 7 + i, h - 3, string.char( key ) )
              nguess = nguess + 1
            end
          end
          if not ok then
            tries = tries + 1
            hang()
          end
        end
      end
      term.moveto( 9, h - 2 )
    end
  end
  if key == term.KC_ESC then break end 
  
  term.moveto( 2, h - 1 )
  total = total + 1
  if nguess == #lword then
    term.print( "Congratulations! Another game? (y/n)" )
    guessed = guessed + 1
  else
    term.print( 8, h - 3, lword )
    term.print( 2, h - 1, "Game over. Another game? (y/n)" )
  end
  
  -- Show statistics again
  stats()
  
  repeat
    key = string.char( term.getchar() ):lower()
  until key == 'y' or key == 'n'
    
  if key == 'n' then 
    break
  end
  
end

term.clrscr()
term.moveto( 1 , 1 )

