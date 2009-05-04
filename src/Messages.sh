#! /usr/bin/env bash
$EXTRACTRC `find . -name \*.rc` >> rc.cpp || exit 11
$EXTRACTRC `find . -name \*.ui` >> rc.cpp || exit 12
$EXTRACTRC `find . -name \*.kcfg` >> rc.cpp || exit 13
$XGETTEXT *.cpp -o $podir/mathematik.pot
rm -f *.cpp
