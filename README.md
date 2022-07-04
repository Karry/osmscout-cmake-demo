# osmscout cmake demo

 - dependency: [libosmscout](https://github.com/Framstag/libosmscout)

This demo tool just lookup peaks around some point and print them.

## Build:
```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=<libosmscout-install-dir> ..
make -j $(nproc)
```

## Usage:

```bash
./peaks ~/Maps/europe-czech-republic-20220514-202336 50.0903431 14.4387094
```
