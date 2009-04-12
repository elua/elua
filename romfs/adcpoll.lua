
if pd.board() == "ET-STM32" then
  timer = 2
  adcchannels = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
  adcsmoothing = {4, 4, 4, 4, 16, 16, 16, 16, 32, 32, 32, 32, 64, 128, 64, 128}
else
  timer = 0
  adcchannels = {0,1,2,3}
  adcsmoothing = {4, 64, 32, 16}
end

for i, v in ipairs(adcchannels) do
  adc.setblocking(v,0)
  adc.setsmoothing(v,adcsmoothing[i])
  adc.setclock(v, 4 ,timer)
end

adc.sample(adcchannels,128)

term.clrscr()

term.gotoxy(1,1)
term.putstr("ADC Status:")
term.gotoxy(1,3)
term.putstr(" CH   SLEN   RES")
term.gotoxy(1,#adcchannels+5)
term.putstr("Press ESC to exit.")

local sample = adc.sample
local getsample = adc.getsample
local samplesready = adc.samplesready
local i, v
local tsample

while true do
  for i, v in ipairs(adcchannels) do
    tsample = getsample(v)
    if not (tsample == nil) then
    	term.gotoxy(1,i+3)
    	term.putstr(string.format("ADC%02d (%03d): %04d\n", v, adcsmoothing[i], tsample))
    end
    if adc.isdone(v) == 1 then adc.sample(v,128) end
  end
  key = term.getch( term.NOWAIT )
  if key == term.KC_ESC then break end
end

term.clrscr()
term.gotoxy(1, 1)
