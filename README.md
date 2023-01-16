# TinyBase

Set of libraries with basic functionalities for C/C++. The goal of them is to help build programs faster, without wasting time redoing the functionalities here on every project.

It also aims at providing low LoC count, zero-dependencies, zero-allocations, platform-independent libraries that can be included in any existing C/C++ codebase. Does not make use of C or C++ standard libraries, with the exception of `memcpy` and `memset` (required by some platform code).

The libraries included are:

* [tinybase-types.h](src/tinybase-types.h): General types and defines used in the other libraries.
* [tinybase-memory.h](src/tinybase-memory.h): Work with memory regions on byte-exact intervals.
* [tinybase-strings.h](src/tinybase-strings.h): String lib for working with different encodings and Unicode.
* [tinybase-queue.h](src/tinybase-queue.h): Thread-safe lists and queues for working with multithreaded code.
* [tinybase-platform.h](src/tinybase-platform.h): API for manipulating system resources (filesystem, IO, threads etc.)

## How to use?

The header files contain the struct and function declarations, and the .c files contain the implementations.

The library files can be moved in whole to the project directory, in which case the header files will include the .c files in the translation unit. They have very short compile times, which makes this approach feasible.

Alternatively, one can build these into objects or static library, in which case passing `TT_STATIC_LINKING` as a preprocessing symbol when compiling the project will prevent the header files from including the implementation files.

## Tests

Unit tests are provided in the /tests/ subfolder for most functionalities in the libraries. Tests for a particular function are not provided when its success or failure cannot be determined in a unit test (e.g. atomic operations).

To build the tests, run the `build.bat` file located in /tests/; a /build/ folder will be created (if not already), and the test executables will be placed there, along with debug symbols.

## License

MIT open source license.