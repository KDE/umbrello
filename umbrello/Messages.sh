#! /bin/sh
#$PREPARETIPS > tips.cpp
$EXTRACTRC *.rc codegenerators/*.ui dialogs/*.ui > ./rc.cpp || exit 11
$XGETTEXT *.cpp *.h -o $podir/umbrello.pot
rm -f tips.cpp
