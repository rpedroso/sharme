parent-id: documentation
submenu-position: 2
title: build
---

Dependencies
------------

* FLTK
* jpeglib
* cmake
* X11 (linux)
* XTest (linux)

Build it
--------

Assuming all builds from a linux 64 bit host.

Build linux 32 bit binary:
-------------------------

    $ cd <to_source_dir>
    $ mkdir bld_lin32
    $ cd bld_lin32
    $ export CFLAGS=-m32 CXXFLAGS=-m32
    $ cmake ..
    $ make

Build linux 64 bit binary:
-------------------------

    $ cd <to_source_dir>
    $ mkdir bld_lin64
    $ cd bld_lin64
    $ # if you have set CFLAGS and CXXFLAGS above, for 32 bit unset them first
    $ unset CFLAGS CXXFLAGS
    $ cmake ..
    $ make


Build windows 32 bit binary
---------------------------

Install mingw32 and the require dependencies for sharme
I recommend [MinGW cross compiling environment](http://mingw-cross-env.nongnu.org/)

    $ cd <to_source_dir>
    $ # edit cmake/toolchain-mingw32.cmake to match your mingw environment, pay attention to
    $ # CMAKE_C_COMPILER, CMAKE_CXX_COMPILER and CMAKE_FIND_ROOT_PATH vars.
    $ mkdir bld_win32
    $ cd bld_win32
    $ cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchain-mingw32.cmake ..
    $ make


Optional
--------
You can strip and compress the binary:

    $ strip sharme(.exe)
    $ upx -9 sharme(.exe)

