#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc` >> rc.cpp || exit 11
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 12
$EXTRACTRC `find . -name \*.kcfg` >> rc.cpp || exit 13
$XGETTEXT `find . -name \*.cpp` -o $podir/cantor.pot
rm -f rc.cpp
