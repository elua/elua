require("LM3S")

disp.init(1000000)
disp.clear()

adc.setsmoothing(0,4)
adc.setsmoothing(1,16)
adc.setsmoothing(2,32)
adc.setsmoothing(3,64)

disp.stringdraw( "ADC Scope", 10, 10, 11 )

adcvals = {}
ctr = 0

while ( true ) do
  ctr = ctr + 1
  
  stime = tmr.start(0)
  adcvals[0] = adc.sample(0)
  adcvals[1] = adc.sample(1)
  adcvals[2] = adc.sample(2)
  adcvals[3] = adc.sample(3)
  etime = tmr.read(0)
  dtime = tmr.diff(0,etime,stime)
  
  if ( ctr == 100 ) then
    ctr = 0
    outstring = string.format("ADC0   (4): %04d",adcvals[0])
    disp.stringdraw( outstring, 10, 10, 11 )
    outstring = string.format("ADC1  (16): %04d",adcvals[1])
    disp.stringdraw( outstring, 10, 20, 11 )
    outstring = string.format("ADC2  (32): %04d",adcvals[2])
    disp.stringdraw( outstring, 10, 30, 11 )
    outstring = string.format("ADC3  (64): %04d",adcvals[3])
    disp.stringdraw( outstring, 10, 40, 11 )
    outstring = string.format("Tcyc: %06d (us)",dtime)
    disp.stringdraw( outstring, 10, 50, 11 )
  end
end