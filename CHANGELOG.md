# Changelog

## 19.12

### New features

    * Add ability to change entry type via context menu

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
