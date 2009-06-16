function error_handler (message)
	io.write ("MY ERROR: " .. message .. "\n");
end

rpc.on_error (error_handler);

slave,err = rpc.connect ("/dev/tty.usbserial-ftCYPMYJ");
--slave,err = rpc.connect ("/dev/ttys0");

function mirror( input )
	return input
end

print(slave.tostring(3.1))

slave.mirror = mirror

for i=1,100000 do math.sqrt(i) end
assert(slave.mirror(42) == 42, "integer return failed")

print(slave.mirror(42))

-- print(slave.mirror:get())

print(slave.collectgarbage("count"))

adc = slave.adc

adc.setblocking(0,1)
adc.setclock(0, 10 ,2)

adc.sample(0,128)
while 1 do
	retsamp = adc.getsample(0)
	if not (retsamp == nil) then
		print(retsamp)
	else
		print(slave.collectgarbage("count"))
		adc.sample(0,128)
	end
end