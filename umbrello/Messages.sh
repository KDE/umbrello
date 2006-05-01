#! /bin/sh
$PREPARETIPS > tips.cpp
$EXTRACTRC *.rc codegenerators/*.ui dialogs/*.ui > ./rc.cpp || exit 11
LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`
if test -n "$LIST"; then
$XGETTEXT $LIST rc.cpp -o $podir/umbrello.pot
fi
rm -f tips.cpp
