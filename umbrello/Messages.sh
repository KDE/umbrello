#! /bin/sh
#$PREPARETIPS > tips.cpp
$EXTRACTRC *.rc codegenerators/*.ui dialogs/*.ui >> ./rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.cpp` -o $podir/umbrello.pot
rm -f tips.cpp
