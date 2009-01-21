require("LM3S")

disp.init(1000000)
disp.clear()

adc.setmode(0,0)

while ( true ) do
  adc.start(0)
  adcval = adc.sample(0)
  outstring = string.format("ADC0: %04d",adcval)
  disp.stringdraw( outstring, 10, 10, 11 )
end