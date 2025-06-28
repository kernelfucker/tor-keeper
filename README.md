# tor-keeper
system-wide tor keeper/tunnel

# compile
$ clang tor-keeper.c -o tor-keeper

# usage
important: do not forget to backup the **torrc** you are using

note: this program must be run as root

$ ./tor-keeper --start --strict

$ ./tor-keeper --force-stop

etc..

# options
```
  --start		start tor with transparent proxy on keeper
  --stop		stop tor and reset firewal
  --force-stop		force kill tor without resetting firewal
  --status		show current tor status and few informatio
  --strict		enable strict nodes selectio
  --random		use random exit node
  --exit-nodes X	set custom exit nodes, comma-separate
```
# example
![image](https://github.com/user-attachments/assets/e0b82019-5abb-43e3-bd02-b5bb5d23686a)
