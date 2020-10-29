## What this project is about
This project is only academic!  
This program allows you to mount different types of archives read-only.  
The implementation is based on libfuse and libarchive. To run it you need this two installed.

## How to install  
### Debian based distros
```
sudo apt install libarchive-dev libfuse-dev pkg-config
cd /path/to/repository
make install
```

### Other systems
For other os you can build and install from source [libarchive](https://github.com/libarchive/libarchive) and [libfuse](https://github.com/libfuse/libfuse) or use your package manager to aquire them  
Then you just have to build and install it  `make install`

### Usage
`archmount [options] <mountpoint> <archive_path>`
