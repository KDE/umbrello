#!/bin/sh
#
# Make a release from the current branches/KDE/3.5/kdesdk/umbrello
#
# Run this script as follows:
#   ./make-umbrello-release.sh VERSION KDEUSER [BRANCH_VERSION]
# VERSION is the version to release.
# KDEUSER is your KDE SVN user name.
# BRANCH_VERSION defaults to 3.5.
# @todo Create release from trunk if BRANCH_VERSION not given.
#       Note: trunk uses the cmake based build process.
#
# The script creates a directory, /tmp/kdesdk, which is used
# as the work area for building the release.
# The release tarfile will be placed in the current working dir.
# 
if [ $# -lt 2 ]; then
  echo "usage:"
  echo "  ./make-umbrello-release.sh VERSION KDEUSER [BRANCH_VERSION]"
  exit 1
fi
version=$1
user=$2
branchver=3.5
if [ $# -gt 2 ]; then
  branchver=$3
fi
origdir=`pwd`
udir=umbrello-$version
svnroot=svn+ssh://${user}@svn.kde.org:/home/kde/branches/KDE/$branchver
cd /tmp
svn co -N $svnroot/kdesdk
cd kdesdk
svn co $svnroot/kdesdk/scripts
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
kdesdk/scripts/svn2dist kdesdk $udir -n umbrello --admin-dir kdesdk/$udir/admin -o
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

