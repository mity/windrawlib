
# WinDrawLib

Home: http://github.com/mity/windrawlib

WinDrawLib is C library for Windows for easy-to-use painting with Direct2D
or, on older Windows versions, GDI+.

The WinDrawLib was created by isolating and cleaning a helper painting code
from the [mCtrl](http://mctrl.org) project as it seems to be very useful on its
own.


## Building WinDrawLib

To build WinDrawLib you need to use CMake to generate MS Visual Studio solution
or to generate Makefile. Then build that normally using your tool chain.

Static lib of WinDrawLib is built as well as few examples using the library.


## Using WinDrawLib

Use WinDrawLib as a normal static library. There is single public header file,
`wtl.h` in the include directory which you need to #include in your sources.

API is documented directly in the public header.

Then link with the static library `WINDRAWLIB.LIB` (Visual Studio) or
`LIBWINDRAWLIB.A` (gcc).


## Release Cycle and API Stability

Note the API is not considered rock-stable and it may be in a constant flux,
although hopefully it will converge to something more or less stable in near
future.

Also there is no typical release process and no official binary packages.
The library is quite small and simple and the intended use is to directly
embed it into a client project.
