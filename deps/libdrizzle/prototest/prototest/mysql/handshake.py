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
MySQL Protocol Handshake Objects
'''

import struct
import unittest
import bitfield

class Capabilities(bitfield.BitField):
  _fields = [
    'LONG_PASSWORD',
    'FOUND_ROWS',
    'LONG_FLAG',
    'CONNECT_WITH_DB',
    'NO_SCHEMA',
    'COMPRESS',
    'ODBC',
    'LOCAL_FILES',
    'IGNORE_SPACE',
    'PROTOCOL_41',
    'INTERACTIVE',
    'SSL',
    'IGNORE_SIGPIPE',
    'TRANSACTIONS',
    'RESERVED',
    'SECURE_CONNECTION',
    'MULTI_STATEMENTS',
    'MULTI_RESULTS',
    None,
    None,
    None,
    None,
    None,
    None,
    None,
    None,
    None,
    None,
    None,
    None,
    'SSL_VERIFY_SERVER_CERT',
    'REMEMBER_OPTIONS'
  ]

class Status(bitfield.BitField):
  _fields = [
    'IN_TRANS',
    'AUTOCOMMIT',
    'MORE_RESULTS_EXISTS',
    'QUERY_NO_GOOD_INDEX_USED',
    'QUERY_NO_INDEX_USED',
    'CURSOR_EXISTS',
    'LAST_ROW_SENT',
    'DB_DROPPED',
    'NO_BACKSLASH_ESCAPES',
    'QUERY_WAS_SLOW'
  ]

class ServerHandshake(object):
  '''This class represents the initial handshake sent from server to client.'''

  def __init__(self, packed=None, protocol_version=10, server_version='',
               thread_id=0, scramble=tuple([0] * 20), null1=0, capabilities=0,
               charset=0, status=0, unused=tuple([0] * 13), null2=0):
    if packed is None:
      self.protocol_version = protocol_version
      self.server_version = server_version
      self.thread_id = thread_id
      self.scramble = scramble
      self.null1 = null1
      self.capabilities = Capabilities(capabilities)
      self.charset = charset
      self.status = Status(status)
      self.unused = unused
      self.null2 = null2
    else:
      self.protocol_version = struct.unpack('B', packed[:1])[0]
      server_version_length = packed[1:].index('\x00')
      self.server_version = packed[1:1+server_version_length]
      data = struct.unpack('<I8BB2BB2B13B12BB', packed[2+server_version_length:])
      self.thread_id = data[0]
      self.scramble = data[1:9] + data[28:40]
      self.null1 = data[9]
      self.capabilities = Capabilities(data[10] | (data[11] << 8))
      self.charset = data[12]
      self.status = Status(data[13] | (data[14] << 8))
      self.unused = data[15:28]
      self.null2 = data[40]

  def pack(self):
    data = struct.pack('B', self.protocol_version)
    data += self.server_version + '\x00'
    data += struct.pack('<I', self.thread_id)
    data += ''.join(map(chr, self.scramble[:8]))
    data += struct.pack('B2BB2B',
                       self.null1,
                       self.capabilities.value() & 0xFF,
                       (self.capabilities.value() >> 8) & 0xFF,
                       self.charset,
                       self.status.value() & 0xFF,
                       (self.status.value() >> 8) & 0xFF)
    data += ''.join(map(chr, self.unused))
    data += ''.join(map(chr, self.scramble[8:]))
    data += struct.pack('B', self.null2)
    return data

  def __str__(self):
    return '''ServerHandshake
  protocol_version = %s
  server_version = %s
  thread_id = %s
  scramble = %s
  null1 = %s
  capabilities = %s
  charset = %s
  status = %s
  unused = %s
  null2 = %s
''' % (self.protocol_version, self.server_version, self.thread_id,
       self.scramble, self.null1, self.capabilities, self.charset,
       self.status, self.unused, self.null2)

class TestServerHandshake(unittest.TestCase):

  def testDefaultInit(self):
    handshake = ServerHandshake()
    self.verifyDefault(handshake)
    handshake.__str__()

  def testKeywordInit(self):
    handshake = ServerHandshake(protocol_version=11,
                                server_version='test',
                                thread_id=1234,
                                scramble=tuple([5] * 20),
                                null1=1,
                                capabilities=65279,
                                charset=253,
                                status=64508,
                                unused=tuple([6] * 13),
                                null2=2)
    self.verifyCustom(handshake)
    handshake.__str__()

  def testUnpackInit(self):
    data = struct.pack('B', 11)
    data += 'test\x00'
    data += struct.pack('<I', 1234)
    data += ''.join([chr(5)] * 8)
    data += struct.pack('B2BB2B', 1, 255, 254, 253, 252, 251)
    data += ''.join([chr(6)] * 13)
    data += ''.join([chr(5)] * 12)
    data += struct.pack('B', 2)

    handshake = ServerHandshake(data)
    self.verifyCustom(handshake)

  def testPack(self):
    handshake = ServerHandshake(ServerHandshake().pack())
    self.verifyDefault(handshake)

  def verifyDefault(self, handshake):
    self.assertEqual(handshake.protocol_version, 10)
    self.assertEqual(handshake.server_version, '')
    self.assertEqual(handshake.thread_id, 0)
    self.assertEqual(handshake.scramble, tuple([0] * 20))
    self.assertEqual(handshake.null1, 0)
    self.assertEqual(handshake.capabilities.value(), 0)
    self.assertEqual(handshake.charset, 0)
    self.assertEqual(handshake.status.value(), 0)
    self.assertEqual(handshake.unused, tuple([0] * 13))
    self.assertEqual(handshake.null2, 0)

  def verifyCustom(self, handshake):
    self.assertEqual(handshake.protocol_version, 11)
    self.assertEqual(handshake.server_version, 'test')
    self.assertEqual(handshake.thread_id, 1234)
    self.assertEqual(handshake.scramble, tuple([5] * 20))
    self.assertEqual(handshake.null1, 1)
    self.assertEqual(handshake.capabilities.value(), 65279)
    self.assertEqual(handshake.charset, 253)
    self.assertEqual(handshake.status.value(), 64508)
    self.assertEqual(handshake.unused, tuple([6] * 13))
    self.assertEqual(handshake.null2, 2)

class ClientHandshake(object):
  '''This class represents the client handshake sent back to the server.'''

  def __init__(self, packed=None, capabilities=0, max_packet_size=0, charset=0,
               unused=tuple([0] * 23), user='', scramble_size=0,
               scramble=None, db=''):
    if packed is None:
      self.capabilities = Capabilities(capabilities)
      self.max_packet_size = max_packet_size
      self.charset = charset
      self.unused = unused
      self.user = user
      self.scramble_size = scramble_size
      self.scramble = scramble
      self.db = db
    else:
      data = struct.unpack('<IIB23B', packed[:32])
      self.capabilities = Capabilities(data[0])
      self.max_packet_size = data[1]
      self.charset = data[2]
      self.unused = data[3:]
      packed = packed[32:]
      user_length = packed.index('\x00')
      self.user = packed[:user_length]
      packed = packed[1+user_length:]
      self.scramble_size = ord(packed[0])
      if self.scramble_size == 0:
        self.scramble = None
      else:
        self.scramble = tuple(map(ord, packed[1:21]))
      if packed[-1:] == '\x00':
        self.db = packed[21:-1]
      else:
        self.db = packed[21:]

  def pack(self):
    data = struct.pack('<IIB', 
                       self.capabilities.value(),
                       self.max_packet_size,
                       self.charset)
    data += ''.join(map(chr, self.unused))
    data += self.user + '\x00'
    data += chr(self.scramble_size)
    if self.scramble_size != 0:
      data += ''.join(map(chr, self.scramble))
    data += self.db + '\x00'
    return data

  def __str__(self):
    return '''ClientHandshake
  capabilities = %s
  max_packet_size = %s
  charset = %s
  unused = %s
  user = %s
  scramble_size = %s
  scramble = %s
  db = %s
''' % (self.capabilities, self.max_packet_size, self.charset, self.unused,
       self.user, self.scramble_size, self.scramble, self.db)

class TestClientHandshake(unittest.TestCase):

  def testDefaultInit(self):
    handshake = ClientHandshake()
    self.verifyDefault(handshake)
    handshake.__str__()

  def testKeywordInit(self):
    handshake = ClientHandshake(capabilities=65279,
                                max_packet_size=64508,
                                charset=253,
                                unused=tuple([6] * 23),
                                user='user',
                                scramble_size=20,
                                scramble=tuple([5] * 20),
                                db='db')
    self.verifyCustom(handshake)
    handshake.__str__()

  def testUnpackInit(self):
    data = struct.pack('<IIB', 65279, 64508, 253)
    data += ''.join([chr(6)] * 23)
    data += 'user\x00'
    data += chr(20)
    data += ''.join([chr(5)] * 20)
    data += 'db\x00'

    handshake = ClientHandshake(data)
    self.verifyCustom(handshake)

  def testPack(self):
    handshake = ClientHandshake(ClientHandshake().pack())
    self.verifyDefault(handshake)

  def verifyDefault(self, handshake):
    self.assertEqual(handshake.capabilities.value(), 0)
    self.assertEqual(handshake.max_packet_size, 0)
    self.assertEqual(handshake.charset, 0)
    self.assertEqual(handshake.unused, tuple([0] * 23))
    self.assertEqual(handshake.user, '')
    self.assertEqual(handshake.scramble_size, 0)
    self.assertEqual(handshake.scramble, None)
    self.assertEqual(handshake.db, '')

  def verifyCustom(self, handshake):
    self.assertEqual(handshake.capabilities.value(), 65279)
    self.assertEqual(handshake.max_packet_size, 64508)
    self.assertEqual(handshake.charset, 253)
    self.assertEqual(handshake.unused, tuple([6] * 23))
    self.assertEqual(handshake.user, 'user')
    self.assertEqual(handshake.scramble_size, 20)
    self.assertEqual(handshake.scramble, tuple([5] * 20))
    self.assertEqual(handshake.db, 'db')

if __name__ == '__main__':
  unittest.main()
