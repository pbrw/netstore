# netstore
Client-server application for distributed file storage written in C++.

This is only a documentation, for more details look at [my webiste](http://pbrw.pl/projects/netstore/).

## Build
[Boost](https://www.boost.org/) program options library is needed to build the application (for parsing command line arguments).

```
git clone https://github.com/pbrw/netstore
cd netstore/src
make
```

## Run server

**Flags**
- ``-g multicast_address`` required
- ``-p udp_port`` required
- ``-f shared_folder_path`` required
- ``-b max_disc_space`` optional, default is 52428800 bytes
- ``-t timeout`` optional, defautl is 5 second


**Example:**
```
mkdir shared
./netstore-server -g 239.0.0.0 -p 8000 -f shared/
```

## Run client

**Flags**
- ``-g multicast_address`` required
- ``-p udp_port`` required
- ``-f out_folder_path`` required
- ``-t timeout`` optional, defautl is 5 second


**Example:**
```
mkdir output
./netstore-client -g 239.0.0.0 -p 8000 -o output/
```

## Client commands

- ``discover`` - displays all available servers
```
discover
Found 192.168.0.227 (239.0.0.0) with free space 52428800
```

- ``upload %s`` - uploads a file to the server cloud, ``%s`` is a file path
```
upload file.txt
File file.txt uploaded (192.168.0.227:53415)
```

- ``search %s`` - lists all the file names that contain ``%s`` (possibly empty)
```
search le
file.txt (192.168.0.227)
```

- ``fetch %s`` - downloads a file with name ``%s`` and saves to output folder
```
search
data.txt (192.168.0.227)
file.txt (192.168.0.227)
fetch data.txt
File data.txt downloaded (192.168.0.227:51195)
```


- ``remove %s`` - deletes a file from the server cloud, ``%s`` is a file name
```
remove file.txt
```

- ``exit`` - closes application












