## xDD

Program for finding waste on user's disk drives. Even huge hard disk drives with terrabyte volume and more have to be cleaned some time. xDD shows you the biggest and unused files, which you want to delete:).

### Dependencies
- CMake
- Qt
- YamlCpp (http://code.google.com/p/yaml-cpp/)

### Platforms
Uses universal Qt-code for scanning disk drives and optimized code for Windows.

Tested on Windows and MSVS2010 compiler with Qt 4.7.4.

It's try to be ready on any other Qt platform.

### Compiling

**Tip**.
Before using CMake on Windows you have to set Qt installation directory in QTDIR environment variable if you've use Qt from archive somewhere.
Use this gist as hint https://gist.github.com/1515211 . Also you have to add YamlCpp path to Cmake YAMLCPP_* variables.

If you installed Qt from installer you might not use this gist.
