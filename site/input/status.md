title: status
menu-position: 1
---

In its current implementation the platforms supported are linux
(should work in almost any recent distribution) and windows (XP/Vists/7)

The limited tests cases I've made, as they cover my real needs, are:

* using a portuguese keyboard layout.
* remote access through LAN and WAN.
  Assuming (Local OS) -> (Remote OS):
    * Linux -> Window
    * Linux -> Linux

Of couse I'm planning do some more as time permits.

***

> ### IMPORTANT
I did not put any efforts on security concerns.
At the moment I only encrypt the desktop image data using arc4
with a hard coded key.

