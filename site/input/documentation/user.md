parent-id: documentation
submenu-position: 0
title: user
---

First, there is no need to install it, it will simply run.


Supose that you want to support the user Bob.
Know both start the app, and you will have to
click in "Remote Screen" button and pass your
IP to Bob. Bob will enter your IP and than
click on "Sharing".

For now Bob must be able to reach to you directly,
so you cannot be behind routers, firewalls.
In the future I'm planning to have the alternative of
sharme talks HTTP(S) and have a server, open source also,
to relay the communication between both ends.

Other option is if you are a Windows Active Directory
administrator and need to support one of your users,
you can do in a windows box command prompt:

    > cd to_where_is_sharme.exe
    > psexec.exe \\<remotepc_ip_or_name> -cf -h -i -u domain\administrator sharme.exe <your_ip>

Before the psexec command above you will need to already be
listen for the incoming connection, you do this by running
sharme in "Remote Screen" mode.

