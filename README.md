# phode

php + libuv = win

## status

Very alpha. And not in the dominating male / leader of the pack sense.

## how to build

Make sure you have the PHP 5.4 header files installed somewhere.

    $ cp local.gypi.example local.gypi
    $ vi local.gypi # update include paths
    $ tools/gyp/gyp -Dlibrary=shared_library
    $ make

Add the following line to your `php.ini`.

    extension = /path/to/phode/out/Debug/obj.target/libphode.so

Now try out some of the examples in the `test/` directory.

    $ php test/test-tcp-ping-pong.php
    Segmentation fault.

The beauty of open source!
