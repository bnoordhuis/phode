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

import unittest

class BitField(object):
  '''Base class for managing bitfields.'''

  _fields = []

  def __init__(self, value=0):
    self._value = value

  def __getattr__(self, name):
    try:
      if name.isupper():
        return 1 << self._fields.index(name)
      elif name.islower():
        return self._value & (1 << self._fields.index(name.upper())) != 0
      raise Exception()
    except Exception:
      raise AttributeError("'%s' object has no attribute '%s'" % (self.__class__.__name__, name))

  def __setattr__(self, name, value):
    try:
      if name[0] == '_':
        self.__dict__[name] = value
      else:
        if name.islower():
          if value:
            self._value |= (1 << self._fields.index(name.upper()))
          else:
            self._value &= ~(1 << self._fields.index(name.upper()))
        else:
          raise Exception()
    except Exception:
      raise AttributeError("'%s' object has no attribute '%s'" % (self.__class__.__name__, name))

  def __str__(self):
    return str([(self._fields[x], 1 << x) for x in range(0, len(self._fields)) if (1 << x) & self._value])

  def value(self):
    return self._value

class ExampleField(BitField):
  _fields = [
    'READ',
    'WRITE',
    'CREATE',
    'DIRECT'
  ]

class TestField(unittest.TestCase):

  def testDefaultInit(self):
    f = ExampleField()
    self.assertEqual(f.value(), 0)

  def testDataInit(self):
    f = ExampleField(15)
    self.assertEqual(f.value(), 15)

  def testGetAttr(self):
    f = ExampleField(1)
    self.assertEqual(f.read, True)
    self.assertEqual(f.READ, 1)
    self.assertEqual(f.write, False)
    self.assertEqual(f.WRITE, 2)

  def testBadGetAttr(self):
    f = ExampleField()
    self.assertRaises(AttributeError, getattr, f, 'BAD')
    self.assertRaises(AttributeError, getattr, f, 'bad')
    self.assertRaises(AttributeError, getattr, f, 'Read')

  def testSetAttr(self):
    f = ExampleField()
    self.assertEqual(f.read, False)
    self.assertEqual(f.write, False)
    f.read = True
    self.assertEqual(f.read, True)
    self.assertEqual(f.write, False)

  def testBadSetAttr(self):
    f = ExampleField()
    self.assertRaises(AttributeError, setattr, f, 'BAD', 0)
    self.assertRaises(AttributeError, setattr, f, 'bad', 0)
    self.assertRaises(AttributeError, setattr, f, 'Read', 0)
    self.assertRaises(AttributeError, setattr, f, 'READ', 0)

if __name__ == '__main__':
  unittest.main()
