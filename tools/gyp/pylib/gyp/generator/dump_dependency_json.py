#!/usr/bin/python

# Copyright (c) 2011 Google Inc. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import collections
import gyp
import gyp.common
import json

generator_wants_static_library_dependencies_adjusted = False

generator_default_variables = {
  'OS': 'linux',
}
for dirname in ['INTERMEDIATE_DIR', 'SHARED_INTERMEDIATE_DIR', 'PRODUCT_DIR',
                'LIB_DIR', 'SHARED_LIB_DIR']:
  # Some gyp steps fail if these are empty(!).
  generator_default_variables[dirname] = 'dir'
for unused in ['RULE_INPUT_PATH', 'RULE_INPUT_ROOT', 'RULE_INPUT_NAME',
               'RULE_INPUT_EXT',
               'EXECUTABLE_PREFIX', 'EXECUTABLE_SUFFIX',
               'STATIC_LIB_PREFIX', 'STATIC_LIB_SUFFIX',
               'SHARED_LIB_PREFIX', 'SHARED_LIB_SUFFIX',
               'LINKER_SUPPORTS_ICF']:
  generator_default_variables[unused] = ''


def CalculateVariables(default_variables, params):
  generator_flags = params.get('generator_flags', {})
  default_variables['OS'] = generator_flags.get('os', 'linux')


def GenerateOutput(target_list, target_dicts, data, params):
  # Map of target -> list of targets it depends on.
  edges = {}

  # Queue of targets to visit.
  targets_to_visit = target_list[:]

  while len(targets_to_visit) > 0:
    target = targets_to_visit.pop()
    if target in edges:
      continue
    edges[target] = []

    for dep in target_dicts[target].get('dependencies', []):
      edges[target].append(dep)
      targets_to_visit.append(dep)

  filename = 'dump.json'
  f = open(filename, 'w')
  json.dump(edges, f)
  f.close()
  print 'Wrote json to %s.' % filename
