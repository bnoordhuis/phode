{
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
        '../php',
        '../php/main',
        '../php/tsrm',
        '../php/zend',
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
      ],
    }
  ] # end targets
}