local disp = lm3s.disp

disp.init( 1000000 )
disp.clear()

local math = math
local floor = math.floor
local pi = math.pi

local img = io.open( "/rom/logo.bin", "rb")
if img == nil then
  print "Unable to load image"
  return 0
end
local imgdata = img:read( "*a" )
img:close()

local maxx , maxy = 64, 64 
local xf, yf = maxx / 2 - 1, maxy / 2 - 1
local ct = yf * maxx + xf + 1

local function rotate( imgdata, angle )
  local c = math.cos( angle )
  local s = math.sin( angle )
  local newdata = bitarray.new( maxx * maxy, 4 )
  local xc, ys = -xf * c, -yf * s
  local xs, yc = -xf * s, -yf * c
  local p1, p2 = xc - ys, xs + yc
  local p3, p4 = -xc - ys, -xs + yc
  local xx1, yy1, xx2, yy2
  local widx1, widx2 = 1, 2*xf + 1
  local widx3, widx4 = 2 * yf * maxx + 1, 2 * yf * maxx + 2 * xf + 1
  local w1, w2, w3, w4
  for y = -yf, 0 do
    xx1, yy1, xx2, yy2 = p1, p2, p3, p4
    w1, w2, w3, w4 = widx1, widx2, widx3, widx4
    for x = -xf, 0 do      
      if ( xx1 >= -xf ) and ( xx1 <= xf ) and ( yy1 >= -yf ) and ( yy1 <= yf ) then 
        newdata[ w1 ] = imgdata[ floor( yy1 ) * maxx + floor( xx1 ) + ct ] 
      end
      if ( xx2 >= -xf ) and ( xx2 <= xf ) and ( yy2 >= -yf ) and ( yy2 <= yf ) then 
        newdata[ w2 ] = imgdata[ floor( yy2 ) * maxx + floor( xx2 ) + ct ] 
      end
      if ( -xx2 >= -xf ) and ( -xx2 <= xf ) and ( -yy2 >= -yf ) and ( -yy2 <= yf ) then 
        newdata[ w3 ] = imgdata[ floor( -yy2 ) * maxx + floor( -xx2 ) + ct ] 
      end
      if ( -xx1 >= -xf ) and ( -xx1 <= xf ) and ( -yy1 >= -yf ) and ( -yy1 <= yf ) then 
        newdata[ w4 ] = imgdata[ floor( -yy1 ) * maxx + floor( -xx1 ) + ct ]
      end
      xx1 = xx1 + c ; yy1 = yy1 + s ; xx2 = xx2 - c ; yy2 = yy2 - s
      w1 = w1 + 1 ; w2 = w2 - 1 ; w3 = w3 + 1 ; w4 = w4 - 1       
    end
    p1 = p1 - s ; p2 = p2 + c  ; p3 = p3 - s ; p4 = p4 + c
    widx1 = widx1 + maxx ; widx2 = widx2 + maxx ; widx3 = widx3 - maxx ; widx4 = widx4 - maxx
  end
  return bitarray.tostring( newdata, "raw" )
end

local origx = ( 128 - maxx ) / 2
local origy = ( 96 - maxy ) / 2
disp.draw( imgdata, origx, origy, maxx, maxy )
local imgd = bitarray.new( imgdata, 4 )
imgdata = nil
collectgarbage()

local delta = pi / 8
local angles = { 0, delta, pi / 4, 3 * delta, pi / 2, 5 * delta, 3 * pi / 4, 7 * delta, pi, 
                 9 * delta, 5 * pi / 4, 11 * delta, 3 * pi / 2, 13 * delta, 7 * pi / 4, 15 * delta }
local index = 2

while uart.getchar( 0, uart.NO_TIMEOUT ) == "" do
  local newimg = rotate( imgd, angles[ index ] )
  disp.draw( newimg, origx, origy, maxx, maxy )
  newimg = nil
  collectgarbage()
  index = index + 1
  index = index > #angles and 1 or index
end

disp.clear()

