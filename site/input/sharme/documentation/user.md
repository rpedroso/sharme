parent-id: documentation
submenu-position: 0
title: user
---

First, there is no need to install it, it will simply run.

This is what it will look like:

> ![sharme screen](/sharme/sharme.png)

Supose that you want to support remote user Bob.
Both ends will start the app. Then make sure
that both you and Bob have the same key code.
One of you need to pass to the other the key.
The key used can be the randonly generated or
what ever you want. This key code is used to
encrypt/decrypt all communications.
You will then hit "start" button and tell Bob
to select "share my screen" option, then fill
the text field "connect to" with your IP and
finally, Bob will hit "start" button.

For now Bob must be able to reach to you directly,
so you cannot be behind routers, firewalls.
In the future I'm planning to have the alternative of
sharme talks HTTP(S) and have a server, that you will need to setup,
to relay the communication between both ends.

Other option is if you are a Windows Active Directory
administrator and need to support one of your users,
you can do in a windows box command prompt, for eg:

    > cd to_where_is_sharme
    > psexec.exe \\<remotepc_ip_or_name> -cf -h -i -u domain\administrator sharme.exe -se <your_ip> -q 40 -key 123

***

> Command line usage is only supported for the client
end - the one that will be sharing the desktop.
type sharme -h for help.

