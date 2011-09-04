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
MySQL Protocol Column Objects
'''

import struct
import unittest
import bitfield
import packet

class ColumnType(object):
  DECIMAL = 0
  TINY = 1
  SHORT = 2
  LONG = 3
  FLOAT = 4
  DOUBLE = 5
  NULL = 6
  TIMESTAMP = 7
  LONGLONG = 8
  INT24 = 9
  DATE = 10
  TIME = 11
  DATETIME = 12
  YEAR = 13
  NEWDATE = 14
  VARCHAR = 15
  BIT = 16
  NEWDECIMAL = 246
  ENUM = 247
  SET = 248
  TINY_BLOB = 249
  MEDIUM_BLOB = 250
  LONG_BLOB = 251
  BLOB = 252
  VAR_STRING = 253
  STRING = 254
  GEOMETRY = 255

class ColumnFlags(bitfield.BitField):
  _fields = [
    'NOT_NULL',
    'PRI_KEY',
    'UNIQUE_KEY',
    'MULTIPLE_KEY',
    'BLOB',
    'UNSIGNED',
    'ZEROFILL',
    'BINARY',
    'ENUM',
    'AUTO_INCREMENT',
    'TIMESTAMP',
    'SET',
    'NO_DEFAULT_VALUE',
    'ON_UPDATE_NOW',
    'PART_KEY',
    'NUM',
    'UNIQUE',
    'BINCMP',
    'GET_FIXED_FIELDS',
    'IN_PART_FUNC',
    'IN_ADD_INDEX',
    'RENAMED'
  ]


class Column(object):
  '''This class represents a column packet sent from the client.'''

  def __init__(self, packed=None, catalog='', db='', table='', orig_table='',
               name='', orig_name='', unused1=0, charset=0, size=0, type=0,
               flags=0, decimal=0, unused2=tuple([0] * 2), default_value=''):
    if packed is None:
      self.catalog = catalog
      self.db = db
      self.table = table
      self.orig_table = orig_table
      self.name = name
      self.orig_name = orig_name
      self.unused1 = unused1
      self.charset = charset
      self.size = size
      self.type = type
      self.flags = ColumnFlags(flags)
      self.decimal = decimal
      self.unused2 = unused2
      self.default_value = default_value
    else:
      self._packed = packed
      self.catalog = self.parseString()
      self.db = self.parseString()
      self.table = self.parseString()
      self.orig_table = self.parseString()
      self.name = self.parseString()
      self.orig_name = self.parseString()
      data = struct.unpack('B2B4BB2BB', self._packed[:11])
      self.unused1 = data[0]
      self.charset = data[1] | (data[2] << 8)
      self.size = data[3] | (data[4] << 8) | (data[5] << 16) | (data[6] << 24)
      self.type = data[7]
      self.flags = ColumnFlags(data[8] | (data[9] << 8))
      self.decimal = data[10]
      self.unused2 = tuple(data[11:13])
      self.default_value = self._packed[13:]

  def parseString(self):
    (size, packed_size) = packet.parse_encoded_size(self._packed)
    string = self._packed[packed_size:size+packed_size]
    self._packed = self._packed[size+packed_size:]
    return string

  def pack(self):
    return chr(self.command) + self.payload

  def __str__(self):
    return '''Column
  catalog = %s
  db = %s
  table = %s
  orig_table = %s
  name = %s
  orig_name = %s
  unused1 = %s
  charset = %s
  size = %s
  type = %s
  flags = %s
  decimal = %s
  unused2 = %s
  default_value = %s
''' % (self.catalog, self.db, self.table, self.orig_table, self.name,
       self.orig_name, self.unused1, self.charset, self.size, self.type,
       self.flags, self.decimal, self.unused2, self.default_value)

if __name__ == '__main__':
  unittest.main()
