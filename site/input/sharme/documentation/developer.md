parent-id: documentation
submenu-position: 1
title: developer
---

sharme in its current form has a simple codebase.
I tried to go with [ravioli code][1], and overall I think I have
achieve that.
Some parts, however, has some [spaghetti code][1], especially
the src/viewer.cpp

[1]: http://en.wikipedia.org/wiki/Spaghetti_code

The way it works is:
--------------------

1. Take a desktop screenshot in rgba
2. If the screen size is not multiple of 16
   (any or both width, height) do a resize
   to make it multiple of 16
3. convert the screenshot image data from
   rgb to yuv420p and at the same time strip
   the alpha channel.
4. Feed the video codec. The codec used is
   the smoke codec from Gstreamer. Gstreamer is
   not needed, I just shameless stole it from
   Gstreamer.
5. Then just send the encoded data to the network.
   Before sent it an encryption is done with arc4


The way it is organized:
------------------------

lib/\* - where several routines are implemented.
        Generally a specific task is in its own .c
        file, for example conversion from rgb to yuv
        and vice versa are in the colorspace.c
include/\* - the headers of the lib/\*

src/viewer.cpp - the code that runs in the supporter
                 machine

src/client.cpp - the code that runs in the machine
                 that it's sharing its screen.

src/sharme.cpp - the main where all starts running.

---

> Source code is hosted on github [https://github.com/rpedroso/sharme](https://github.com/rpedroso/sharme) so go fork it.

