# Assignment 2: Building a VM Autoscaler.

## Dependencies

- libgtkmm3.0
- libgtk3
- libcairo2
- libcairomm
- libpango
- libpangocairo
- libglibmm-2.4
- libvirt


## Directory Structure

The `src` directory cntains the source code for the **client**, **server** and **vm manager** in their respective folders.

The `build-release` folder contains the executables and build files for the corresponding client, server and vm manager in thier respective folders.

The `testcases` folder contains a script to generate input that can be fed to the Client.

```
193059004
├── Readme.md
├── src
│   ├── client
│   │   ├── Client.cpp
│   │   ├── CMakeLists.txt
│   │   └── KVClientLibrary.cpp
│   ├── server
│   │   ├── Server.cpp
│   │   ├── KVStore.cpp
│   │   ├── KVCache.cpp
│   │   └── ThreadPool.cpp
│   │   ├── KVClientHandler.cpp
│   │   ├── KVServerLibrary.cpp
│   │   ├── CMakeLists.txt
│   └── VMManager
│       ├── main.cpp
│       ├── VM.h
│       ├── VM.cpp
│       ├── Graph.h
│       ├── Graph.cpp
│       ├── Manager.h
│       ├── Manager.cpp
│       └── VmManager.h
│       ├── VmManager.cpp
│       ├── CMakeLists.txt
├── testcases
│   ├── input0.csv
│   └── testcase_generator.py
└── build-release
    ├── client
    │   └── ...
    ├── manager
    │   └── ...
    └── server
        └── ...
```


## Compilation Instructions

This project has been build using `cmake` build system, which can be installed on Ubuntu using,

		sudo apt-get install cmake

In order to build the project, cd into the `server` , `client` or `manager` directory under `build-release` directory,correspondingly whichever needs to be built.

Empty the build directory if compiling on a new machine for the first time. For example, if manager needs to be built, the build directory for manager which is *193059004/build-release/manager*, first remove the contents of this directory using, `rm -rf 193059004/build-release/manager/.`


Once the build folder is cleaned up `cd` into it and type,
		

		cmake <path to source directory>
	
This will create all the necessary make files, required to build the executable from the source files. The, just run `make` to build the executable.

For example,
```
cd 193059004/build-release/manager
rm -rf .
cmake ../../src/VMManager
make
```

This will build the executable for Manager.


Alternatively, the executable for the corresponding source, the executable will already be present in the corresponding build directory.
