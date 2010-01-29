-- Acquire ADC samples using a timer with polling for available samples

if pd.board() == "ET-STM32" then
  timer = 2
  adcchannels = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
  adcsmoothing = {4, 4, 4, 4, 16, 16, 16, 16, 32, 32, 32, 32, 64, 128, 64, 128}
else
  timer = 0
  adcchannels = {0,1,2,3}
  adcsmoothing = {4, 64, 32, 16}
end

-- Setup ADC and start sampling
for i, v in ipairs(adcchannels) do
  adc.setblocking(v,0) -- no blocking on any channels
  adc.setsmoothing(v,adcsmoothing[i]) -- set smoothing from adcsmoothing table
  adc.setclock(v, 4 ,timer) -- get 4 samples per second, per channel
end

-- Draw static text on terminal
term.clrscr()
term.print(1,1,"ADC Status:")
term.print(1,3," CH   SLEN   RES")
term.print(1,#adcchannels+5,"Press ESC to exit.")

-- start sampling on all channels at the same time 
adc.sample(adcchannels,128) 

while true do
  for i, v in ipairs(adcchannels) do
    -- If samples are not being collected, start
    if adc.isdone(v) == 1 then adc.sample(v,128) end 
    
    -- Try and get a sample
    tsample = adc.getsample(v)
    
    -- If we have a new sample, then update display
    if not (tsample == nil) then 
    	term.print(1,i+3,string.format("ADC%02d (%03d): %04d\n", v, adcsmoothing[i], tsample))
    end
  end
  
  -- Exit if user hits Escape
  key = term.getchar( term.NOWAIT )
  if key == term.KC_ESC then break end 
end

term.clrscr()
term.moveto(1, 1)
