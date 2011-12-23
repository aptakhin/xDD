## xDD

Program for finding waste on user's disk drives. Even huge hard disk drives with terrabyte volume and more have to be cleaned some time. xDD shows you the biggest and unused files, which you can delete.

### Dependencies
- CMake
- Qt

### Platforms
Uses universal Qt-code for scanning disk drives and optimized code for Windows.

Tested on Windows and MSVS2010 compiler with Qt 4.7.4.

Tryes to be ready on any other platform.

### Compiling

**Tip**.
Before using CMake on Windows you have to set Qt installation directory in QTDIR environment variable. 
Use this gist as hint (msvc10_generate.bat)[https://gist.github.com/1515211]
