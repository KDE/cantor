# Changelog

## 20.04

### New features

    * WorksheetControlItem: special element, for better UX, while user interact with cell. Now, this element handle drag-and-drop and cell selection (details below)
    * Multiple cells selection: now user can select not one, but many sells via Ctrl+LeftClick. Selection also visualizate on control elements
    * Actions on selection (first version): now user can apply some actions on selected cells, for example evaulating, deleting, moving.
    * Possibility to change result collapsing via double click on '>>>' prompt element
    * Add collapsing of text results with a lot of visible lines (limit of collapsing set in Settings). Double click can collapse/uncollapse collapsed text result.

## 19.12

### Screenshots

    * https://imgur.com/frfNeBH
    * https://imgur.com/IAJ4YAN
    * https://imgur.com/eBesNdR

### New features

    * Support for Jupyter Notebook format (.ipynb)
    * Allow to convert Cantor's native worksheet format to Jupyter notebook and back
    * Allow to change the type of a worksheet entry via the context menu
    * Leave the markdown and latex cells in the edit mode if the the user hits the cancel/escape button
    * Add opportunity to set path to local documentation in Sage backend

### Important bug fixes

    * Fix rendering of embedded math in Markdown Entry on openSUSE
    * Show the pointer hand cursor when hovering over a URL in a markdown entry
    * Make sagemath backend compatible with sagemath built with python3

## 19.08

### Screenshots

    * https://imgur.com/Xpj2EcQ
    * https://imgur.com/KnXYvFP
    * https://imgur.com/CmucWdR

### New features

    * Instead of showing only available and workable backends, Cantor shows all available backends and for non workable shows reason, why this backend doesn't work.
    * Allow to set the path to custom Julia installations. However, Cantor will work with versions only it was compiled for.
    * For Markdown and LaTeX entries allow to switch via double click from the rendered result to the original code and back via the evaluation of the entry
    * Save the results of rendered markdown and LaTeX entries as part of the project. This allows to see the results also on with no support for markdown and latex rendering
    * Hide "Help" panel on startup. Automatically show this panel when user executes a command entry with a help expression
    * Add "Recent Files" submenu (https://bugs.kde.org/show_bug.cgi?id=409138)

### Important bug fixes

    * [R] Fix bug with expression only from comment - now Cantor R backend don't freeze on 'Computing' after running the expression
    * Save error status and message of Command Entry into .cws (Cantor Worksheet file) - Cantor have lost them on saving before
    * Reset Command Entry numeration after Backend restart
    * Close loaded worksheet, if the loading failed (before Cantor show empty broken worksheet)
    * [Python] Fix bug with non-working interruption (before interrupted only Cantor expression: Python Server still continued to work)
    * Don't scroll to worksheet's end after the project was loaded
    * [Julia, Python] Report about server side errors, for example, crashes
    * [Python] Don't use Qt in pythonserver executable for avoding problems (often crashes) with PyQt5 (https://bugs.kde.org/show_bug.cgi?id=397264, https://bugs.kde.org/show_bug.cgi?id=407362)
    * [Python] Show Python warnings not as errors, but as text results (https://bugs.kde.org/show_bug.cgi?id=409240)
    * Add missing context menu to MarkdownEntry
    * Fix bug with rendering loaded rendered MarkdownEntry as empty
    * Fix unworking 'Show LaTeX Code' action in Latex Entry context menu
    * Fix problem with an incorrect window title after closing all tabs

## 19.04

### New Features

    * Possibility to hide and show results of command entry via context menu
    * [Maxima, Octave, Python, R] Add a way to specify the path to the local documentation in the settings. By default, this path didn't specified and Cantor uses online documentation.
    * Huge improvment of variables managment: better parsing, option to turn on and turn off variable managment, better GUI performance by limitation of value size (too big for showing values replaced by "<too big variable>" text

### Important bug fixes

    * [Sage] Fix execuation for unsystem Sage installation
    * [Julia] Fix bug, that suppressing output don't work
