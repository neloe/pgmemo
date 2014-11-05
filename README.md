# pgmemo
This package strives to be a way to memoize compilcated postgres queries in redis assuming one knows the output from postgresql will not change.

## Prerequisites
This program requires the hiredis C bindings for redis and libpqxx, as well as the [FastBSON-Cpp](github.com/neloe/FastBSON-Cpp) and [EasyZMQ-Cpp](github.com/neloe/EasyZMQ-Cpp) library.  The prerequisites on a Debian/Ubuntu system may be installed with
```bash
sudo apt-get install libpqxx-dev libhiredis-dev
```