-- Acquire ADC samples as quickly as possible, without the use of a timer
--  provides statistics on time and memory usage while running

if pd.board() == "ET-STM32" then
  adcchannels = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
  adcsmoothing = {4, 4, 4, 4, 16, 16, 16, 16, 32, 32, 32, 32, 64, 128, 64, 128}
  numiter = 50
elseif pd.board() == "MBED" then
  adcchannels = {0,1,2,3,4,5}
  adcsmoothing = {4, 4, 16, 16, 32, 32}
  numiter = 2
else
  adcchannels = {0,1,2,3}
  adcsmoothing = {4, 16, 64, 128}
  numiter = 200
end

-- Setup ADC
for i, v in ipairs(adcchannels) do
  adc.setblocking(v,1) -- block, waiting for samples when an adc.get* function is used
  adc.setclock(v,0) -- set clock to zero: no timer, acquire samples as fast as possible
  adc.setsmoothing(v,adcsmoothing[i]) -- apply moving average filter using lengths from adcsmoothing
end

-- Draw static text on terminal
term.clrscr()
term.print(1,1,"ADC Status:")
term.print(1,3," CH   SLEN   RES")
term.print(1,#adcchannels+7,"Press ESC to exit.")

-- Use some locals for speed
local adcvals = {}
local key, stime, etime, dtime, i, v
local sample = adc.sample
local insertsamples = adc.insertsamples
local tread = tmr.read
local tstart = tmr.start

while true do
  stime = tstart(0) -- start timer
  for j=1,numiter do -- acuire numiter samples
    sample(adcchannels, 1)
    for i, v in ipairs(adcchannels) do
      insertsamples(v,adcvals,i,1) -- for each iteration j, get samples and put them in adcvals
    end
  end
  etime = tread(0) -- get cycle end time
  dtime = tmr.gettimediff(0,etime,stime)/numiter -- compute average acquisition time per cycle
  
  -- draw last acquired samples on the console
  term.moveto(1,4)
  for i, v in ipairs(adcchannels) do
    term.print(string.format("ADC%02d (%03d): %04d\n", v, adcsmoothing[i],adcvals[i]))
    term.moveto(1,i+4)
  end
  
  -- draw acquisition statistics
  term.print(string.format("Tcyc: %06d (us)\n",dtime))
	term.print(1,#adcchannels+5,string.format("Mem:  %03.2f (kB)\n",collectgarbage("count")))

  key = term.getchar( term.NOWAIT )
  if key == term.KC_ESC then break end -- exit if user hits Escape
end

term.clrscr()
term.moveto( 1 , 1 )
