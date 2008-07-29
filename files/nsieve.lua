-- The Computer Language Shootout
-- http://shootout.alioth.debian.org/
-- contributed by Isaac Gouy
-- modified by Mike Pall


local function nsieve(m,isPrime)
   for i=2,m do
      isPrime[i] = true
   end
   local count = 0

   for i=2,m do
      if isPrime[i] then
         for k=i+i, m, i do
--            if isPrime[k] then isPrime[k] = false end
            isPrime[k] = false
         end
         count = count + 1
      end
   end
   return count
end


local n = tonumber(arg and arg[1]) or 2
if n < 2 then n = 2 end
local flags = {}

local m = (2^n)*10000
io.write( string.format("Primes up to %8d %8d", m, nsieve(m,flags)), "\n")

m = (2^(n-1))*10000
io.write( string.format("Primes up to %8d %8d", m, nsieve(m,flags)), "\n")

m = (2^(n-2))*10000
io.write( string.format("Primes up to %8d %8d", m, nsieve(m,flags)), "\n")

--[[
COMMAND LINE (%A is single numeric argument):

 /usr/bin/lua $LUA_EXTRAS ../nsieve.lua-3.lua %A


PROGRAM OUTPUT
==============
Primes up to  5120000   356244
Primes up to  2560000   187134
Primes up to  1280000    98610
]]
