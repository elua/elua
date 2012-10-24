import decoder
import sys, os
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection

import numpy as np
import matplotlib.pyplot as plt

################################################################################

ex = open( sys.argv[ 2 ], "r" )
ef = ELFFile( ex )

ftable = []

""" Display the symbol tables contained in the file
"""
for section in ef.iter_sections():
  if not isinstance(section, SymbolTableSection):
    continue

  if section['sh_entsize'] == 0:
    continue

  for nsym, symbol in enumerate(section.iter_symbols()):
    # symbol names are truncated to 25 chars, similarly to readelf
    if symbol[ 'st_info' ][ 'type' ] != 'STT_FUNC':
      continue
    t = { "name" : symbol.name, "start" : symbol[ 'st_value' ], "size" : symbol[ 'st_size' ]  }
    t[ "end" ] = t[ "start" ] + t[ "size" ]
    ftable.append( t )

ftable.append( { "name" : "<0>", "start" : 0, "size" : 0, "end" : 0 } )
ftable = sorted( ftable, key = lambda e: e[ "start" ] )

def funcsearch(addr):
  lo, hi = 0, len( ftable )
  while lo < hi:
    mid = ( lo + hi ) // 2
    midval = ftable[ mid ]
    start = midval[ "start" ]
    if start <= addr < midval[ "end" ]:
      return midval
    elif start < addr:
      lo = mid + 1
    else:
      hi = mid
  return ftable[ 0 ]

################################################################################

f = open( sys.argv[ 1 ], "rb" )
s = f.read()
f.close()

d = decoder.Decoder( s )
#while True:
#  ( idx, offset, op, lvl, data ) = d.decode_next()
#  if idx is None:
#    break
#  if op == d.FREE:
#    trace = data.get( "trace", None )
#    if trace:
#      stk = 0
#      for t in trace:
#        afrom, ato = t[ "from" ], t[ "to" ]
#        nfrom, nto = funcsearch( afrom ), funcsearch( ato )
#        print( "  [%02d] %s (%08X) [from %s + 0x%X (%08X)]" % ( len( trace ) - stk, nto[ "name" ], ato, nfrom[ "name" ], afrom - nfrom[ "start" ], afrom ) )
#        stk = stk + 1
#      break

def print_trace( trace ):
  if trace:
    stk = 0
    for t in trace:
      afrom, ato = t[ "from" ], t[ "to" ]
      nfrom, nto = funcsearch( afrom ), funcsearch( ato )
      print( "  [%02d] %s (%08X) [from %s + 0x%X (%08X)]" % ( len( trace ) - stk, nto[ "name" ], ato, nfrom[ "name" ], afrom - nfrom[ "start" ], afrom ) )
      stk = stk + 1
  else:
    print( "<no trace available>" )

def handle_malloc( lvl, idx, data ):
  global alloc_addrs, alloc_sizes, alloc_size_map, totsize, maxsize, alloc_traces
  try:
    alloc_addrs.find( data[ "ptr" ] )
  except:
    pass
  else:
    print "ADDRESS ALREADY PRESENT?!"
  l = data[ "len" ]
  if l == 4:
    print( "ALLOC FOR4:" )
    print_trace( data.get( "trace", None ) )
  alloc_addrs.append( data[ "ptr" ] )
  alloc_sizes.append( l )
  alloc_traces.append( data.get( "trace", None ) )
  if not alloc_size_map.has_key( l ):
    alloc_size_map[ l ] = 1
  else:
    alloc_size_map[ l ] = alloc_size_map[ l ] + 1
  totsize += l
  maxsize = max( totsize, maxsize )

def handle_free( lvl, opidx, data ):
  global alloc_addrs, alloc_sizes, alloc_size_map, totsize, maxsize, alloc_traces
  try:
    idx = alloc_addrs.index( data[ "ptr" ] )
  except ValueError:
    if data[ "ptr" ] != 0:
      print( "ERROR? %08X" % data[ "ptr" ] )
      print_trace( data.get( "trace", None ) )
    return
  bsize = alloc_sizes[ idx ]
  alloc_addrs = alloc_addrs[ :idx ] + alloc_addrs[ idx + 1: ]
  alloc_sizes = alloc_sizes[ :idx ] + alloc_sizes[ idx + 1: ]
  alloc_traces = alloc_traces[ :idx ] + alloc_traces[ idx + 1: ]
  totsize -= bsize

def handle_realloc( lvl, idx, data ):
  global alloc_addrs, alloc_sizes, alloc_size_map, totsize, maxsize
  oldp, newp, size = data[ "oldptr" ], data[ "newptr" ], data[ "len" ]
  trace = data.get( "trace", None )
  if size == 0: # free
    handle_free( lvl, idx, { "ptr" : oldp, "trace" : trace } )
  elif oldp == 0: # malloc
    handle_malloc( lvl, idx, { "ptr" : newp, "len" : size, "trace" : trace } )
  else: # actual realloc (handle as free + malloc)
    handle_free( lvl, idx, { "ptr" : oldp, "trace" : trace } )
    handle_malloc( lvl, idx, { "ptr" : newp, "len": size, "trace" : trace } )

for i in xrange( 1 ):
  alloc_addrs = []
  alloc_sizes = []
  alloc_traces = []
  alloc_size_map = {} 
  nallocs, nfrees, nreallocs = 0, 0, 0
  recording = False
  tlvl = 2
  totsize, maxsize = 0, 0
  while True:
    ( idx, offset, op, lvl, data ) = d.decode_next()
    if op == d.START and lvl == tlvl:
      print( "Starting analysis for '%s'" % data[ "msg" ] )
      recording = True
    elif op == d.STOP and lvl == tlvl:
      break
    if recording:
      if op == d.MALLOC:
        handle_malloc( lvl, idx, data )
        nallocs = nallocs + 1
      elif op == d.FREE:
        handle_free( lvl, idx, data )
        nfrees = nfrees + 1
      elif op == d.REALLOC:
        handle_realloc( lvl, idx, data )
        nreallocs = nreallocs + 1

  print( "ALLOCS:    ", nallocs )
  print( "FREES:     ", nfrees )
  print( "REALLOCS:  ", nreallocs )
  print( "TOTSIZE:   ", totsize )
  print( "MAXSIZE:   ", maxsize )

  def plotit( xx, y ):
    N = len( xx )
    x = np.arange(0, N)
    labels = xx
    width = 1
    bar1 = plt.bar( x, y, width, color="y" )
    plt.ylabel( 'Allocations' )
    plt.xticks(x + width / 2.0, labels )
    plt.show()

  xlist, ylist = [], []
  for k in sorted( alloc_size_map.keys() ):
    xlist.append( k )
    ylist.append( alloc_size_map[ k ] )

  plotit( xlist, ylist )

  print( "*" * 80 )
  print( "Remaining allocations" )
  print( "*" * 80 )
  t = 0
  for idx, addr in enumerate( alloc_addrs ):
    print( "%08X (%d bytes)" % ( addr, alloc_sizes[ idx ] ) )
    if alloc_traces[ idx ]:
      print_trace( alloc_traces[ idx ] )
    t += alloc_sizes[ idx ]

  print( "total not reclaimed: %d" % t )
