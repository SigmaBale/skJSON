# skJson
Cache friendly standard compliant Json parser written in ANSI C.

Create, serialize, parse and safely modify JSON files while at all times preserving
the correct JSON standard.
(Potentially standard breaking functions are suffixed `_unsafe`)

### Build
Clone the repository and cd into it:
```
git clone git@github.com:SigmaBale/skJSON.git
cd skJSON
```
There are two ways to build, using **CMake** (more portable) and **GNU Make** (with gcc).

#### CMake
Build using **CMake** in this order:
```
mkdir build
cd build
cmake ..
make
```

#### GNU Make
Build using **GNU Make**:
```
make
```
### Install
Default install directories: 

**UNIX**
- header file:  `/usr/local/include`
- library: `/usr/local/lib`

**Windows**
- header file: `C:\Program Files\skJSON\include`
- library: `C:\Program Files\skJSON\lib`

This assumes you have already built the project and
are located in the project source directory.

#### CMake
```
cd build
sudo make install
```
If you wish to change the default installation path
then you can change prefix like this (ignore '<' and '>'):
```
sudo cmake --install . --prefix <your-prefix>
```
For example if you were to run this:
```
sudo cmake --install . --prefix /home/myusername/projects
```
Then the header file/s would be installed in `/home/myusername/projects/include`,
and the project library in `/home/myusername/projects/lib`.

#### GNU Make
```
make install
```
If you wish to change the installation path open the `Makefile` and edit the
`HEADER_INSTALL` and `LIB_INSTALL` variables to your desired path. Of course
`HEADER_INSTALL` meaning installation path for header file and `LIB_INSTALL`
installation path for shared library.

### Testing
Assuming that you are in the projects source directory.
Make sure to add the shared library to the linker path so the
test executable can be run.

Also when you install and add the library to the known linker path
run `sudo ldconfig` in terminal.

#### CMake
This will build the tests:
```
cd build
cmake -D SKJSON_BUILD_TESTS=1 ..
make
```
This will run them:
```
ctest
```
or this:
```
make test
```
To run with memory checker (default valgrind):
```
ctest -T memcheck
```
NOTE: Next time you run `cmake ..` it will remove the user variables from the cache.
In order to build again with tests enabled you have to pass `cmake -D SKJSON_BUILD_TESTS ..`
again and do `make`.

#### GNU Make
This will build the tests and run them:
```
make test
```
If you wish to run test with **sanitizer** enabled (*address and undefined*):
```
make test-sanitizer
```
or you can run with **valgrind** (*memory leak full*):
```
make test-valgrind
```
