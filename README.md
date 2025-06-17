# tor-keeper
system-wide tor keeper/tunnel

# compile
$ clang tor-keeper.c -o tor-keeper

# usage
note: this program must be run as root

$ ./tor-keeper --start --strict

$ ./tor-keeper --force-stop

etc..

# options
--start\               start tor with transparent proxy on keeper

--stop                stop tor and reset firewall

--force-stop          force kill tor without resetting firewall

--status              show current tor status and few information

--strict              enable strict nodes selection

--random              use random exit nodes

--exit-nodes X        set custom exit nodes, comma-separated

# example
![image](https://github.com/user-attachments/assets/33fa45e7-9a91-4c61-aa54-87582b1df265)
