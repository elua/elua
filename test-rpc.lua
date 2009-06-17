function error_handler (message)
	io.write ("MY ERROR: " .. message .. "\n");
end

rpc.on_error (error_handler);

slave,err = rpc.connect ("/dev/tty.usbserial-ftCYPMYJ");
--slave,err = rpc.connect ("/dev/ttys0");

function mirror( input ) return input end
function squareval(x) return x^2 end
test_local = {1, 2, 3, 3.143, "234"}
test_local.sval = 23

slave.mirror = mirror

-- reflect parameters off mirror
-- also requires that function serialization works
assert(slave.mirror(42) == 42, "integer return failed")
assert(slave.mirror("this is a test!") == "this is a test!", "string return failed")
assert(string.dump(slave.mirror(squareval)) == string.dump(squareval), "function return failed")
assert(slave.mirror(true) == true, "bool return failed")

slave.test = test_local

-- get remote table
assert(slave.test:get(), "couldn't get remote table")


-- check that we can get entry on remote table
assert(test_local.sval == slave.test:get().sval, "table field not equivalent")
slave.collectgarbage("collect")
print(slave.collectgarbage("count"))

adc = slave.adc

adc.setblocking(0,1)
adc.setclock(0, 10 ,2)

adc.sample(0,128)
print(slave.collectgarbage("count"))
while 1 do
	retsamp = adc.getsample(0)
	if not (retsamp == nil) then
		print(retsamp)
	else
		print(slave.collectgarbage("count"))
		adc.sample(0,128)
	end
end