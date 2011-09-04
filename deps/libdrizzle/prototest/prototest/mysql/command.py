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
MySQL Protocol Command Objects
'''

import unittest

class CommandID(object):
  SLEEP = 0
  QUIT = 1
  INIT_DB = 2
  QUERY = 3
  FIELD_LIST = 4
  CREATE_DB = 5
  DROP_DB = 6
  REFRESH = 7
  SHUTDOWN = 8
  STATISTICS = 9
  PROCESS_INFO = 10
  CONNECT = 11
  PROCESS_KILL = 12
  DEBUG = 13
  PING = 14
  TIME = 15
  DELAYED_INSERT = 16
  CHANGE_USER = 17
  BINLOG_DUMP = 18
  TABLE_DUMP = 19
  CONNECT_OUT = 20
  REGISTER_SLAVE = 21
  STMT_PREPARE = 22
  STMT_EXECUTE = 23
  STMT_SEND_LONG_DATA = 24
  STMT_CLOSE = 25
  STMT_RESET = 26
  SET_OPTION = 27
  STMT_FETCH = 28
  DAEMON = 29
  END = 30

class Command(object):
  '''This class represents a command packet sent from the client.'''

  def __init__(self, packed=None, command=CommandID.SLEEP, payload=''):
    if packed is None:
      self.command = command
      self.payload = payload
    else:
      self.command = ord(packed[0])
      self.payload = packed[1:]

  def pack(self):
    return chr(self.command) + self.payload

  def __str__(self):
    return '''Command
  command = %s
  payload = %s
''' % (self.command, self.payload)

class TestCommand(unittest.TestCase):

  def testDefaultInit(self):
    command = Command()
    self.assertEqual(command.command, CommandID.SLEEP)
    self.assertEqual(command.payload, '')

  def testKeywordInit(self):
    command = Command(command=CommandID.QUERY, payload='abc')
    self.assertEqual(command.command, CommandID.QUERY)
    self.assertEqual(command.payload, 'abc')

  def testUnpackInit(self):
    command = Command('\x03abc')
    self.assertEqual(command.command, CommandID.QUERY)
    self.assertEqual(command.payload, 'abc')

  def testPack(self):
    command = Command(Command(command=CommandID.QUERY, payload='abc').pack())
    self.assertEqual(command.command, CommandID.QUERY)
    self.assertEqual(command.payload, 'abc')

class QueryCommand(Command):
  def __init__(self, packed=None, query=''):
    super(QueryCommand, self).__init__(packed=packed, command=CommandID.QUERY,
                                       payload=query)

  def __str__(self):
    return '''Command
  command = %s
  query = %s
''' % (self.command, self.payload)

class TestQueryCommand(unittest.TestCase):

  def testDefaultInit(self):
    query = QueryCommand()
    self.assertEqual(query.command, CommandID.QUERY)
    self.assertEqual(query.payload, '')

  def testKeywordInit(self):
    query = QueryCommand(query='abc')
    self.assertEqual(query.command, CommandID.QUERY)
    self.assertEqual(query.payload, 'abc')

  def testUnpackInit(self):
    query = QueryCommand('\x03abc')
    self.assertEqual(query.command, CommandID.QUERY)
    self.assertEqual(query.payload, 'abc')

  def testPack(self):
    query = QueryCommand(QueryCommand(query='abc').pack())
    self.assertEqual(query.command, CommandID.QUERY)
    self.assertEqual(query.payload, 'abc')

if __name__ == '__main__':
  unittest.main()
