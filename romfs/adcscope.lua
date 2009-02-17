
adcchannels = {0, 1, 2, 3}
adcsmoothing = {4, 16, 32, 64}

for i, v in ipairs(adcchannels) do
  adc.setmode(v,0)
  adc.setsmoothing(v,adcsmoothing[i])
end

term.clrscr()

term.gotoxy(1,1)
term.putstr("ADC Status:")
term.gotoxy(1,3)
term.putstr(" CH   SLEN   RES")
term.gotoxy(1,#adcchannels+6)
term.putstr("Press ESC to exit.")

local adcvals = {}
local ctr = 0

while true do
  local key, stime, etime, dtime
  local sample = adc.sample
  local getsamples = adc.getsamples
  ctr = ctr + 1
  
  stime = tmr.start(0)
  for i, v in ipairs(adcchannels) do
    sample(v)
    adcvals[i] = getsamples(v,1)
  end
  etime = tmr.read(0)
  dtime = tmr.diff(0,etime,stime)
  
  if ( ctr == 100 ) then
    ctr = 0
    term.gotoxy(1,4)
    for i, v in ipairs(adcchannels) do
      term.putstr(string.format("ADC%d (%03d): %04d\n",v,adcsmoothing[i],adcvals[i]))
      term.gotoxy(1,i+4)
    end
    term.putstr(string.format("Tcyc: %06d (us)\n",dtime))
    
    key = term.getch( term.NOWAIT )
    if key == term.KC_ESC then break end
  end
  if key == term.KC_ESC then break end
end

term.clrscr()
term.gotoxy( 1 , 1 )