#!/bin/sh
#
# Make a release from the current branches/KDE/3.5/kdesdk/umbrello
#
# Run this script as follows:
#   . make-umbrello-release.sh [BRANCH_VERSION]
# BRANCH_VERSION defaults to 3.5.
# @todo Create release from trunk if BRANCH_VERSION not given.
#       Note: trunk uses the cmake based build process.
#
# The script creates a directory, /tmp/kdesdk, which is used
# as the work area for building the release.
# The release tarfile will be placed in the current working dir.
# The release version is taken from the VERSION file.
# 
branchver=3.5
if [ $# -gt 1 ]; then
  branchver=$2
fi
origdir=`pwd`
version=`grep "^[1-9]" VERSION`
udir=umbrello-$version
svnroot=svn://anonsvn.kde.org:/home/kde/branches/KDE/$branchver
cd /tmp
svn co -N $svnroot/kdesdk
cd kdesdk
svn co $svnroot/kdesdk/umbrello $udir
svn co $svnroot/kde-common/admin $udir/admin
svn co -N $svnroot/kdesdk/doc $udir/doc
svn co    $svnroot/kdesdk/doc/umbrello $udir/doc/umbrello
cp -p Makefile.cvs $udir/
cd $udir
mv configure.in.in configure.in.in.orig
echo '#MIN_CONFIG'                  > configure.in.in
echo 'KDE_ENABLE_HIDDEN_VISIBILITY' >> configure.in.in
echo 'CXXFLAGS="$CXXFLAGS $KDE_DEFAULT_CXXFLAGS"' >> configure.in.in
echo ''                                           >> configure.in.in
cat configure.in.in.orig                          >> configure.in.in
rm configure.in.in.orig
perl -p -e 's@umbrello/VERSION@VERSION@g' -i `find umbrello -name Makefile.am`
cd /tmp
log=/tmp/kdesdk/svn2dist.log
$origdir/../scripts/svn2dist kdesdk $udir -n umbrello --admin-dir kdesdk/$udir/admin \
         --svn-root svn://anonsvn.kde.org/home/kde/branches/stable --log=$log -o
mv umbrello/po kdesdk/$udir/
rm -rf umbrello
cd kdesdk/$udir
make -f Makefile.cvs
cd ..
tarfile=${udir}.tar.bz2
tar cfvj $tarfile --exclude=.svn --exclude=autom4te.cache $udir
mv $tarfile $origdir/
cd $origdir
# rm -rf /tmp/kdesdk

echo upload $tarfile to upload.sf.net
echo wput $tarfile ftp://upload.sf.net/incoming/
echo update uml.sf.net including uploading ChangeLog
echo advertise on freshmeat and kde-apps

