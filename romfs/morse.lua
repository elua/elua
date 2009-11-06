-------------------------------------------------------------------------------
-- eLua Morse Demo
--
-- Dado Sutter         sep 2008
-------------------------------------------------------------------------------

local pwmid, tmrid, ledpin
if pd.board() == "EK-LM3S8962" or pd.board() == "EK-LM3S6965" then
  wmid, tmrid, ledpin = 1, 1, pio.PF_0
elseif pd.board() == "EAGLE-100" then
  wmid, tmrid, ledpin = 1, 1, pio.PE_1
elseif pd.board() == "SAM7-EX256" then
  pwmid, tmrid, ledpin = 0, 1, pio.PB_20
  tmr.setclock( 1, 100000 )
else
  print( pd.board() .. " not supported with this example" )
  return
end

------------ User Adjusted Variables ------------

local dotDelay, playFreq, playFreqSave = 90000, 880, 880
local freqStep, dotDelayStep = 220, 10000

-- Morse Alphabet
local Morse = {
  A='.-',
  B='-...',
  C='-.-.',
  D='-..',
  E='.',
  F='..-.',
  G='--.',
  H='....',
  I='..',
  J='.---',
  K='-.-',
  L='.-..',
  M='--',
  N='-.',
  O='---',
  P='.--.',
  Q='--.-',
  R='.-.',
  S='...',
  T='-',
  U='..-',
  V='...-',
  W='.--',
  X='-..-',
  Y='-.--',
  Z='--..'
}

------------ Auxiliar Functions ------------

local function play(m)
  term.print(m)
  if m == ' ' then
    tmr.delay(tmrid, 2 * dotDelay)
  else
    pio.pin.sethigh( ledpin )
    pwm.start(pwmid)
    tmr.delay(tmrid, m == '.' and dotDelay or 3 * dotDelay)
    pwm.stop(pwmid)
    pio.pin.setlow( ledpin )
    tmr.delay(tmrid, dotDelay)
  end
end

local function HandleKbd(k)
  if k == term.KC_ESC then
    return true
  elseif k == term.KC_UP then                 -- Speed up
    dotDelay = dotDelay + dotDelayStep
  elseif k == term.KC_DOWN then               -- Speed down
    dotDelay = dotDelay - dotDelayStep
  elseif k > 0 and k < 256 then
    if string.char(k) == '+' then             -- Frequency up
      playFreq = playFreq + freqStep
    elseif string.char(k) == '-' then         -- Frequency down
      playFreq = playFreq - freqStep
    elseif string.char(k) == 's' then         -- Sound on/off
      if playFreq == 0 then
        playFreq = playFreqSave
      else
        playFreqSave = playFreq
        playFreq = 0
      end
    end
  end
  pwm.setup(pwmid, playFreq, 50)
end

------------ Main Program ------------
pio.pin.setdir( pio.OUTPUT, ledpin )
pwm.setup( pwmid, playFreq, 50 )

while true do
  term.clrscr()
  term.moveto(1, 1)
  print("Welcome to eLua Morse Playing on " .. pd.board())
  io.write("Enter phrase (empty phrase to exit): ")
  local msg, enabled = io.read(), true
  if #msg == 0 then break end

  term.print('   ')
  while term.getchar(term.NOWAIT) ~= -1 do end        -- flush

  while enabled do                                  -- Main Loop
    for i = 1, #msg do                              -- msg loop
      local ch = msg:sub(i, i):upper()
      term.print(ch)                               -- show what will be played
      if ch ~= ' ' and Morse[ch] then
        for j = 1, #Morse[ch] do                    -- Morse symbol loop
          play(Morse[ch]:sub(j,j))                  -- play each morse symbol
        end
      else
        play(' ') play(' ')                         -- Between words
      end
      play(' ')                                     -- Extra between words & lett
      key = term.getchar(term.NOWAIT)                 -- Handle UI actions
      if key ~= -1 then
        if HandleKbd(key) then
          enabled = false
          break
        end
      end
    end
    if not enabled then break end
    print()
    play(' ') play(' ') play(' ')                  -- Between sentences
  end
end
