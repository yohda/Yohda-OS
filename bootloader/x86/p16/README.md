For real mode in Yohda OS, there are two mode for debugging.

1" First, you can do debuggig with GDB. 
In the case, you have to pass the argument `D=1` to makefile.
e.g `make D=1`. you can change the value `D` to anything what you want.

2" Second, for protected mode, you just should use only `make` command.
