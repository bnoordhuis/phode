{
  'includes': [
    './common.gypi',
    './local.gypi',
  ],

  'variables': {
    'target_arch': 'ia32',
  },

  'targets': [
    {
      'target_name': 'phode',
      'type': 'shared_library',

      'defines': [
        'ZTS=1',
        'ZEND_DEBUG=1',
        'PHP_DEBUG=1',
      ],

      'dependencies': [
        'deps/http_parser/http_parser.gyp:http_parser',
        'deps/libuv/uv.gyp:uv',
      ],

      'include_dirs': [
        'src',
        'deps/uv/src/ares',
        '<(PHP_INCLUDE_PATH)',
        '<(PHP_INCLUDE_PATH)/main',
        '<(PHP_INCLUDE_PATH)/TSRM',
        '<(PHP_INCLUDE_PATH)/Zend',
      ],

      'sources': [
        'src/ext.c',
        'test.php',
        'gen.bat',
      ],

      'conditions': [
        [ 'OS=="win"', {
            'defines': [ 
              'ZEND_WIN32=1',
              'PHP_WIN32=1',
              'ZEND_WIN32=1',
              'PHP_WIN32=1',
            ],
            'libraries': [ 
              '-l../php/Debug_TS/php5ts_debug.lib' 
            ]
        }],
        [ 'OS=="linux" or OS=="freebsd" or OS=="openbsd" or OS=="solaris"', {
          'cflags': [
            '-std=c99',
            '-fPIC',
          ],
        }],
      ],
    }
  ] # end targets
}
