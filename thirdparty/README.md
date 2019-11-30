## 3rd party libraries

This folder contains (patched) versions of libraries and files Cantor depends on.


## DISCOUNT

DISCOUNT is a implementation of Markdown markup language ([link](https://github.com/Orc/discount)).

The version included here provides two additional patches:

* Better LaTeX support: `$` math delimiter and `mkd_e_latex` callback (https://github.com/Orc/discount/pull/214)

* Better recognition of the mathematical expressions between $...$, $$...$$


## preview.sty
This file provides the LaTeX style 'preview' ([link](https://www.ctan.org/tex-archive/macros/latex/contrib/preview)).

The main purpose of the preview package is the extraction of selected
elements from a LaTeX source, like formulas or graphics, into separate
pages of a DVI file.  A flexible and convenient interface allows it to
specify what commands and constructs should be extracted.  This works
with DVI files postprocessed by either Dvips and Ghostscript or
dvipng, but it also works when you are using PDFTeX for generating PDF
files.

This package is used for the rendering of mathematical LaTeX expressions embedded in Cantor's worksheet.
