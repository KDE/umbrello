#! /bin/sh
#$PREPARETIPS > tips.cpp
$EXTRACTRC *.rc `find . -name \*.ui` *.kcfg >> ./rc.cpp || exit 11
$XGETTEXT `find . -name \*.h -o -name \*.cpp` -o $podir/umbrello.pot
rm -f tips.cpp rc.cpp
