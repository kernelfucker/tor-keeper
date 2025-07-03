# tor-keeper
system-wide tor keeper/tunnel

# compile
$ clang tor-keeper.c -o tor-keeper -Os -s

# usage
important: do not forget to backup the **torrc** you are using

note: this program must be run as root

$ ./tor-keeper -s -c

$ ./tor-keeper -fx

etc..

# options
```
  -s    start tor with transparent proxy on keeper
  -x    stop tor and reset firewal
  -fx   force kill tor without resetting firewal
  -t    show current tor status and few informatio
  -c    enable strict nodes selectio
  -r    use random exit node
  -e X  set custom exit nodes, comma-separate
```
# example
![image](https://github.com/user-attachments/assets/e0b82019-5abb-43e3-bd02-b5bb5d23686a)
