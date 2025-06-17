# tor-keeper
system-wide tor keeper/tunnel

# compile
$ clang tor-keeper.c -o tor-keeper

# usage
important: do not forget to backup the torrc you are using

note: this program must be run as root

$ ./tor-keeper --start --strict

$ ./tor-keeper --force-stop

etc..

# options
--start

--stop

--force-stop

--status

--strict

--random

--exit-nodes

# example
![image](https://github.com/user-attachments/assets/33fa45e7-9a91-4c61-aa54-87582b1df265)
