# Decoder for the memory tracer data stream

import struct

class Decoder:

  ( START, MALLOC, FREE, REALLOC, CALLOC, STOP ) = range( 6 )

  def __init__( self, s ):
    self.data = s
    self.pos = 0
    self.idx = 0

  def rewind( self, pos = 0 ):
    self.pos = 0

  def __next( self, n = 1 ):
    s = self.data[ self.pos : self.pos + n ]
    self.pos += n
    return s

  def __u8( self ):
    return struct.unpack( ">B", self.__next() )[ 0 ]

  def __u16( self ):
    return struct.unpack( ">H", self.__next( 2 ) )[ 0 ]

  def __u32( self ):
    return struct.unpack( ">L", self.__next( 4 ) )[ 0 ]

  def __getstr( self ):
   total = self.__u16()
   return struct.unpack( "%ds" % total, self.__next( total ) )[ 0 ]

  # Return format:
  #   - start: idx, pos, op, lvl, message
  #   - malloc: idx, pos, op, lvl, ptr, len
  #   - free: idx, pos, op, lvl, ptr
  #   - realloc: idx, pos, op, lvl, newptr, oldptr, len
  #   - calloc: idx, pos, op, lvl, block_size, nblocks
  #   - stop: idx, pos, op, lvl
  def decode_next( self ):
    if self.pos >= len( self.data ):
      return ( None, None, None, None, None )
    pos = self.pos
    op = self.__u8()
    lvl = self.__u8()
    extra = {}
    if lvl & 0x80:
      # This contains a stack trace
      lvl = lvl & 0x7F
      extra[ "trace" ] = []
      total = self.__u16()
      for i in xrange( 0, total ):
        extra[ "trace" ].append( { "from" : self.__u32(), "to" : self.__u32() } )
    if op == self.START:
      extra[ "msg" ] = self.__getstr()
    elif op == self.MALLOC:
      extra[ "ptr" ] = self.__u32()
      extra[ "len" ] = self.__u32()
    elif op == self.FREE:
      extra[ "ptr" ] = self.__u32()
    elif op == self.REALLOC:
      extra[ "newptr" ] = self.__u32()
      extra[ "oldptr" ] = self.__u32()
      extra[ "len" ] = self.__u32()
    elif op == self.CALLOC: # convert calloc to malloc for now
      op = self.MALLOC
      extra[ "ptr" ] = self.__u32()
      extra[ "len" ] = self.__u32() * self.__u32()
    elif op == self.STOP:
      pass
    else:
      print( "WARNING: unknown op", op )
    self.idx += 1
    return ( self.idx - 1, pos, op, lvl, extra )

