## Cantor

Cantor is a KDE Application aimed to provide a nice Interface
for doing Mathematics and Scientific Computing. It doesn't implement
its own Computation Logic, but instead is built around different
Backends.

## Available Backends

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
cmake .. -DCMAKE_INSTALL_PREFIX=/path/to/install -DCMAKE_BUILD_TYPE=debugfull
make
make install or su -c 'make install'
```

If `-DCMAKE_INSTALL_PREFIX` is not used, Cantor will be installed in
root directory.

If you provide some `/path/to/install`, you must to configure the
[`QT_PLUGIN_PATH`](http://doc.qt.io/qt-5/deployment-plugins.html#the-plugin-directory)
environment variable in order to Cantor loads the plugins correctly.

To uninstall the project:

```make uninstall or su -c 'make uninstall'```
