## Cantor

Cantor is a KDE Application aimed to provide a nice Interface
for doing Mathematics and Scientific Computing. It doesn't implement
its own Computation Logic, but instead is built around different
Backends.

## Available Backends

- Julia Programming Language: http://julialang.org/
- KAlgebra for Calculation and Plotting: http://edu.kde.org/kalgebra/
- Lua Programming Language: http://lua.org/
- Maxima Computer Algebra System: http://maxima.sourceforge.net/
- Octave for Numerical Computation: https://gnu.org/software/octave/
- Python 2 Programming Language: http://python.org/
- Python 3 Programming Language: http://python.org/
- Qalculate Desktop Calculator: http://qalculate.sourceforge.net/
- R Project for Statistical Computing: http://r-project.org/
- Sage Mathematics Software: http://sagemath.org/
- Scilab for Numerical Computation: http://scilab.org/

## How To Build and Install Cantor

To build and install Cantor, follow the steps below:

```
cd cantor
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install -DCMAKE_BUILD_TYPE=RELEASE
make
make install or su -c 'make install'
```

If `-DCMAKE_INSTALL_PREFIX` is not used, Cantor will be installed in
default cmake install directory (`/usr/local/` usually).
Also, setting `CMAKE_INSTALL_PREFIX` to some unstandart location may happens different problems
with searching Cantor files. So there is a need for passing proper paths for KDE install cmake variables.
For example, if no one Cantor's backends found after installation, KDE_INSTALL_PLUGINDIR path should
be specified on cmake configuration step to some location.

To uninstall the project:

`make uninstall` or `su -c 'make uninstall'`
