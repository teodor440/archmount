## What this project is about
This program lets you mount different types of archives as read-only  
The implementation is based on libfuse and libarchive. To run it you need this two installed  

## How to install  
### Ubuntu
First you should install or have installed [libfuse3](https://github.com/libfuse/libfuse) then preceed to:
```
sudo apt install libarchive-dev  && sudo apt install libfuse-dev && sudo apt install pkg-config
cd /path/to/repository
make install
```  
### Other systems
For other os you can build and install from source [libarchive](https://github.com/libarchive/libarchive) and [libfuse](https://github.com/libfuse/libfuse) or use your package manager to aquire them  
Then you just have to build and install it  

### Usage
`archmount [options] <mountpoint> <archive_path>`
