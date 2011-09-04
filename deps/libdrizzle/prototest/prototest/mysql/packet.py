#!/usr/bin/env python
#
# Drizzle Client & Protocol Library
# 
# Copyright (C) 2008 Eric Day (eday@oddments.org)
# All rights reserved.
# 
# Use and distribution licensed under the BSD license.  See
# the COPYING file in this directory for full text.
#
'''
MySQL Protocol Packet Objects
'''

import struct
import unittest

class PacketException(Exception):
  pass

class Packet(object):
  '''This class represents a packet header.'''

  def __init__(self, packed=None, size=0, sequence=0):
    if packed is None:
      self.size = size
      self.sequence = sequence
    else:
      data = struct.unpack('4B', packed)
      self.size = data[0] | (data[1] << 8) | (data[2] << 16)
      self.sequence = data[3]

    self.verify()

  def pack(self):
    self.verify()
    return struct.pack('4B',
                       self.size & 0xFF,
                       (self.size >> 8) & 0xFF,
                       (self.size >> 16) & 0xFF,
                       self.sequence % 256)

  def verify(self):
    if self.size >= 16777216:
      raise PacketException('Packet size cannot exceed 16777215 bytes (%d)' %
                            self.size)

  def __str__(self):
    return '''Packet
  size = %s
  sequence = %s
''' % (self.size, self.sequence)

class TestPacket(unittest.TestCase):

  def testDefaultInit(self):
    packet = Packet()
    self.assertEqual(packet.size, 0)
    self.assertEqual(packet.sequence, 0)
    packet.__str__()

  def testKeywordInit(self):
    packet = Packet(size=1234, sequence=5)
    self.assertEqual(packet.size, 1234)
    self.assertEqual(packet.sequence, 5)
    packet.__str__()

  def testUnpackInit(self):
    packet = Packet(struct.pack('4B', 210, 4, 0, 5))
    self.assertEqual(packet.size, 1234)
    self.assertEqual(packet.sequence, 5)

  def testPack(self):
    packet = Packet(Packet(size=1234, sequence=5).pack())
    self.assertEqual(packet.size, 1234)

  def testPackRange(self):
    for x in range(0, 300):
      packet = Packet(Packet(size=x, sequence=x).pack())
      self.assertEqual(packet.size, x)
      self.assertEqual(packet.sequence, x % 256)

    # 997 is a random prime number so we hit various increments
    for x in range(300, 16777216, 997):
      packet = Packet(Packet(size=x, sequence=x).pack())
      self.assertEqual(packet.size, x)
      self.assertEqual(packet.sequence, x % 256)

    packet = Packet(Packet(size=16777215).pack())
    self.assertEqual(packet.size, 16777215)
    self.assertEqual(packet.sequence, 0)

    self.assertRaises(PacketException, Packet, size=16777216)
    self.assertRaises(PacketException, Packet, size=16777217)
    self.assertRaises(PacketException, Packet, size=4294967295)
    self.assertRaises(PacketException, Packet, size=4294967296)
    self.assertRaises(PacketException, Packet, size=4294967297)

def parse_row(count, data):
  row = []
  while count > 0:
    count -= 1
    if ord(data[0]) == 251:
      row.append(None)
      data = data[1:]
    else:
      (size, packed_size) = parse_encoded_size(data)
      row.append(data[packed_size:packed_size+size])
      data = data[packed_size+size:]
  return row

class BadSize(Exception):
  pass

def parse_encoded_size(data):
  size = ord(data[0])
  packed_size = 1
  if size == 252:
    size = struct.unpack('<H', data[1:3])[0]
    packed_size = 3
  elif size == 253:
    data = struct.unpack('<HB', data[1:4])
    size = data[0] | (data[1] << 16)
    packed_size = 4
  elif size == 254:
    data = struct.unpack('<II', data[1:9])
    size = data[0] | (data[1] << 32)
    packed_size = 8
  elif size == 255:
    raise BadSize(str(size))

  return (size, packed_size)

if __name__ == '__main__':
  unittest.main()
