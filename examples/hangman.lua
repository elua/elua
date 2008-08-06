-- Hangman in eLua using the 'term' module
-- Inspired by the original 'hangman' from the bsdgames package

local h, w = term.lines(), term.cols()
local tries = 0

-- "Databse" with our words
local words = { "simple", "hangman", "guess", "elua", "inane", "serial", 
  "stupenduous", "software" }

-- Build our hanging site :)
function hang()
  if tries == 0 then
    -- Build the basic structure
    term.gotoxy( 5, 1 )
    term.putstr( string.rep( '_', 6 ) )
    term.gotoxy( 5, 2 )
    term.putstr( '|    |')
    local i
    for i = 3, 6 do
      term.gotoxy( 5, i )
      term.putstr( '|' )
    end
    term.gotoxy( 3, 7 )
    term.putstr( '__|_____')
    term.gotoxy( 3, 8 )
    term.putstr( '|      |___')
    term.gotoxy( 3, 9 )
    term.putstr( '|__________|') 
    
  elseif tries == 1 then
    -- Draw the head
    term.gotoxy( 10, 3 )
    term.putstr( "O" )
    
  elseif tries == 2 or tries == 3 then
    -- First or second part of body
    term.gotoxy( 10, tries + 2 )
    term.putstr( "|" )
    
  elseif tries == 4 or tries == 5 then
    -- First leg / first hand
    term.gotoxy( 9, tries == 4 and 6 or 4 )
    term.putstr( "/" )
  
  elseif tries == 6 or tries == 7 then
    -- Second hand / second leg
    term.gotoxy( 11, tries == 7 and 6 or 4 )
    term.putstr( "\\" )
  end  
end

local total, guessed = 0, 0

-- Show the game statistics
function stats()
  term.gotoxy( w - 20, 5 )
  term.putstr( "Total words: ", tostring( total ) )
  term.gotoxy( w - 20, 6 )
  term.putstr( "Guessed words: ", tostring( guessed ) )
end

while true do
  term.clrscr()
  stats()
  
  -- Draw the hanging site
  tries = 0
  hang()
    
  -- Then write the "Guess" line
  term.gotoxy( 2, h - 3 )
  term.putstr( "Word: " )
  local lword = words[ math.random( #words ) ]:lower()
  term.putstr( string.rep( "-", #lword ) )
  term.gotoxy( 2, h - 2 )
  term.putstr( "Guess: " )
  
  local nguess = 0
  local tried = {}
  local key
  while tries < 7 and nguess < #lword do     
    key = string.char( term.getch( term.WAIT ) ):lower()
    term.gotoxy( 2, h - 1 )
    term.clreol()       
    if not key:find( '%l' ) then
      term.putstr( "Invalid character" )
    else
      key = key:byte()
      if tried[ key ] ~= nil then
        term.putstr( "Already tried this key" )
      else
        tried[ key ] = true
        local i
        local ok = false
        for i = 1, #lword do
          if key == lword:byte( i ) then
            ok = true
            term.gotoxy( 7 + i, h - 3 )
            term.put( key )
            nguess = nguess + 1
          end
        end
        if not ok then
          tries = tries + 1
          hang()
        end
      end
    end
    term.gotoxy( 9, h - 2 )
  end
  
  term.gotoxy( 2, h - 1 )
  total = total + 1
  if nguess == #lword then
    term.putstr( "Congratulations! Another game? (y/n)" )
    guessed = guessed + 1
  else
    term.gotoxy( 8, h - 3 )
    term.putstr( lword )
    term.gotoxy( 2, h - 1 )
    term.putstr( "Game over. Another game? (y/n)" )
  end
  
  -- Show statistics again
  stats()
  
  repeat
    key = string.char( term.getch( term.WAIT ) ):lower()
  until key == 'y' or key == 'n'
    
  if key == 'n' then 
    break
  end
  
end

term.clrscr()
term.gotoxy( 1 , 1 )

