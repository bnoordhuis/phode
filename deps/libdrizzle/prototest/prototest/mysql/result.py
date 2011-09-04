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
MySQL Protocol Result Objects
'''

import struct
import unittest

class BadFieldCount(Exception):
  pass

class OkResult(object):
  '''This class represents an OK result packet sent from the server.'''

  def __init__(self, packed=None, affected_rows=0, insert_id=0, status=0,
               warning_count=0, message='', version_40=False):
    if packed is None:
      self.affected_rows = affected_rows
      self.insert_id = insert_id
      self.status = status
      self.message = message
      self.version_40 = version_40
      if version_40 is False:
        self.warning_count = warning_count
    else:
      self.version_40 = version_40
      if ord(packed[0]) != 0:
        raise BadFieldCount('Expected 0, received ' + str(ord(packed[0])))
      self.affected_rows = ord(packed[1])
      self.insert_id = ord(packed[2])
      if version_40 is True:
        if len(packed) == 3:
          self.status = 0
          self.message = ''
        else:
          data = struct.unpack('<H', packed[3:5])
          self.status = data[0]
          self.message = packed[5:]
      else:
        data = struct.unpack('<HH', packed[3:7])
        self.status = data[0]
        self.warning_count = data[1]
        self.message = packed[7:]

  def __str__(self):
    if self.version_40 is True:
      return '''OkResult
  affected_rows = %s
  insert_id = %s
  status = %s
  message = %s
  version_40 = %s
''' % (self.affected_rows, self.insert_id, self.status, self.message,
       self.version_40)
    else:
      return '''OkResult
  affected_rows = %s
  insert_id = %s
  status = %s
  warning_count = %s
  message = %s
  version_40 = %s
''' % (self.affected_rows, self.insert_id, self.status, self.warning_count,
       self.message, self.version_40)

class TestOkResult(unittest.TestCase):

  def testDefaultInit(self):
    result = OkResult()
    self.assertEqual(result.affected_rows, 0)
    self.assertEqual(result.insert_id, 0)
    self.assertEqual(result.status, 0)
    self.assertEqual(result.warning_count, 0)
    self.assertEqual(result.message, '')
    self.assertEqual(result.version_40, False)
    result.__str__()

  def testDefaultInit40(self):
    result = OkResult(version_40=True)
    self.assertEqual(result.affected_rows, 0)
    self.assertEqual(result.insert_id, 0)
    self.assertEqual(result.status, 0)
    self.assertEqual(result.message, '')
    self.assertEqual(result.version_40, True)
    result.__str__()

  def testKeywordInit(self):
    result = OkResult(affected_rows=3, insert_id=5, status=2,
                      warning_count=7, message='test', version_40=False)
    self.assertEqual(result.affected_rows, 3)
    self.assertEqual(result.insert_id, 5)
    self.assertEqual(result.status, 2)
    self.assertEqual(result.warning_count, 7)
    self.assertEqual(result.message, 'test')
    self.assertEqual(result.version_40, False)

  def testUnpackInit(self):
    data = struct.pack('BBB', 0, 3, 5)
    data += struct.pack('<HH', 2, 7)
    data += 'test'

    result = OkResult(data)
    self.assertEqual(result.affected_rows, 3)
    self.assertEqual(result.insert_id, 5)
    self.assertEqual(result.status, 2)
    self.assertEqual(result.warning_count, 7)
    self.assertEqual(result.message, 'test')
    self.assertEqual(result.version_40, False)
    result.__str__()

  def testUnpackInit40(self):
    data = struct.pack('BBB', 0, 3, 5)
    data += struct.pack('<H', 2)
    data += 'test'

    result = OkResult(data, version_40=True)
    self.assertEqual(result.affected_rows, 3)
    self.assertEqual(result.insert_id, 5)
    self.assertEqual(result.status, 2)
    self.assertEqual(result.message, 'test')
    self.assertEqual(result.version_40, True)
    result.__str__()

class ErrorResult(object):
  '''This class represents an error result packet sent from the server.'''

  def __init__(self, packed=None, error_code=0, sqlstate_marker='#',
               sqlstate='XXXXX', message='', version_40=False):
    if packed is None:
      self.error_code = error_code
      self.message = message
      self.version_40 = version_40
      if version_40 is False:
        self.sqlstate_marker = sqlstate_marker
        self.sqlstate = sqlstate
    else:
      self.version_40 = version_40
      if ord(packed[0]) != 255:
        raise BadFieldCount('Expected 255, received ' + str(ord(packed[0])))
      data = struct.unpack('<H', packed[1:3])
      self.error_code = data[0]
      if version_40 is True:
        self.message = packed[3:]
      else:
        self.sqlstate_marker = packed[3]
        self.sqlstate = packed[4:9]
        self.message = packed[9:]

  def __str__(self):
    if self.version_40 is True:
      return '''ErrorResult
  error_code = %s
  message = %s
  version_40 = %s
''' % (self.error_code, self.message, self.version_40)
    else:
      return '''ErrorResult
  error_code = %s
  sqlstate_marker = %s
  sqlstate = %s
  message = %s
  version_40 = %s
''' % (self.error_code, self.sqlstate_marker, self.sqlstate, self.message,
       self.version_40)

class TestErrorResult(unittest.TestCase):

  def testDefaultInit(self):
    result = ErrorResult()
    self.assertEqual(result.error_code, 0)
    self.assertEqual(result.sqlstate_marker, '#')
    self.assertEqual(result.sqlstate, 'XXXXX')
    self.assertEqual(result.message, '')
    self.assertEqual(result.version_40, False)
    result.__str__()

  def testDefaultInit40(self):
    result = ErrorResult(version_40=True)
    self.assertEqual(result.error_code, 0)
    self.assertEqual(result.message, '')
    self.assertEqual(result.version_40, True)
    result.__str__()

  def testKeywordInit(self):
    result = ErrorResult(error_code=3, sqlstate_marker='@', sqlstate='ABCDE',
                         message='test', version_40=False)
    self.assertEqual(result.error_code, 3)
    self.assertEqual(result.sqlstate_marker, '@')
    self.assertEqual(result.sqlstate, 'ABCDE')
    self.assertEqual(result.message, 'test')
    self.assertEqual(result.version_40, False)
    result.__str__()

  def testUnpackInit(self):
    data = chr(255)
    data += struct.pack('<H', 1234)
    data += '#ABCDE'
    data += 'test'

    result = ErrorResult(data)
    self.assertEqual(result.error_code, 1234)
    self.assertEqual(result.sqlstate_marker, '#')
    self.assertEqual(result.sqlstate, 'ABCDE')
    self.assertEqual(result.message, 'test')
    self.assertEqual(result.version_40, False)
    result.__str__()

  def testUnpackInit40(self):
    data = chr(255)
    data += struct.pack('<H', 1234)
    data += 'test'

    result = ErrorResult(data, version_40=True)
    self.assertEqual(result.error_code, 1234)
    self.assertEqual(result.message, 'test')
    self.assertEqual(result.version_40, True)
    result.__str__()

class EofResult(object):
  '''This class represents an EOF result packet sent from the server.'''

  def __init__(self, packed=None, warning_count=0, status=0, version_40=False):
    if packed is None:
      self.version_40 = version_40
      if self.version_40 is False:
        self.warning_count = warning_count
        self.status = status
    else:
      self.version_40 = version_40
      if ord(packed[0]) != 254:
        raise BadFieldCount('Expected 254, received ' + str(ord(packed[0])))
      if version_40 is False:
        data = struct.unpack('<HH', packed[1:])
        self.warning_count = data[0]
        self.status = data[1]

  def __str__(self):
    if self.version_40 is True:
      return '''EofResult
  version_40 = %s
''' % self.version_40
    else:
      return '''EofResult
  warning_count = %s
  status = %s
  version_40 = %s
''' % (self.warning_count, self.status, self.version_40)

class TestEofResult(unittest.TestCase):

  def testDefaultInit(self):
    result = EofResult()
    self.assertEqual(result.warning_count, 0)
    self.assertEqual(result.status, 0)
    self.assertEqual(result.version_40, False)
    result.__str__()

  def testDefaultInit40(self):
    result = EofResult(version_40=True)
    self.assertEqual(result.version_40, True)
    result.__str__()

  def testKeywordInit(self):
    result = EofResult(warning_count=3, status=5, version_40=False)
    self.assertEqual(result.warning_count, 3)
    self.assertEqual(result.status, 5)
    self.assertEqual(result.version_40, False)
    result.__str__()

  def testUnpackInit(self):
    data = chr(254)
    data += struct.pack('<HH', 3, 5)

    result = EofResult(data)
    self.assertEqual(result.warning_count, 3)
    self.assertEqual(result.status, 5)
    self.assertEqual(result.version_40, False)
    result.__str__()

  def testUnpackInit40(self):
    result = EofResult(chr(254), version_40=True)
    self.assertEqual(result.version_40, True)
    result.__str__()

class CountResult(object):
  '''This class represents an count result packet sent from the server.'''

  def __init__(self, packed=None, count=0):
    if packed is None:
      self.count = count
    else:
      self.count = ord(packed[0])
      if self.count == 0 or self.count > 253:
        raise BadFieldCount('Expected 1-253, received ' + str(ord(packed[0])))

  def __str__(self):
    return '''CountResult
  count = %s
''' % self.count

class TestCountResult(unittest.TestCase):

  def testDefaultInit(self):
    result = CountResult()
    self.assertEqual(result.count, 0)
    result.__str__()

  def testKeywordInit(self):
    result = CountResult(count=3)
    self.assertEqual(result.count, 3)
    result.__str__()

  def testUnpackInit(self):
    result = CountResult("\x03")
    self.assertEqual(result.count, 3)
    result.__str__()

def create_result(packed, version_40=False):
  '''This function creates the appropriate result object instance depending on
     first byte.'''
  count = ord(packed[0])
  if count == 0:
    return OkResult(packed, version_40=version_40)
  if count == 254:
    return EofResult(packed, version_40=version_40)
  if count == 255:
    return ErrorResult(packed, version_40=version_40)
  return CountResult(packed)

if __name__ == '__main__':
  unittest.main()
