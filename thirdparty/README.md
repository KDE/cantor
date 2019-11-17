## 3rd party libraries

This folder contains (patched) versions of libraries and files Cantor depends on.


## DISCOUNT

DISCOUNT is a implementation of Markdown markup language ([link](https://github.com/Orc/discount)).

The version included here provides two additional patches:

* Better LaTeX support: `$` math delimiter and `mkd_e_latex` callback (https://github.com/Orc/discount/pull/214)

* Better recognition of the mathematical expressions between $...$, $$...$$


## standalone.cls
This file provides the LaTeX class and package 'standalone' ([link](https://ctan.org/tex-archive/macros/latex/contrib/standalone)),
which allows TeX pictures or other TeX code in sub-files to be compiled standalone or as part of a main document.
This package is used for the rendering of mathematical LaTeX expressions embedded in the Cantor's worksheet.
