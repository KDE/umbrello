#!/bin/sh
#
# Make a release from the current branches/KDE/3.5/kdesdk/umbrello
#
# Run this script as follows:
#   . make-umbrello-release.sh VERSION [KDEUSER]
# where VERSION is to be replaced by the version to release
# and KDEUSER is to be replaced by your KDE SVN user name
# (only required if you're not jriddell.)
#
# The release tarfile will be placed in /tmp/kdesdk.
# 
version=$1
user=$2
if [ "X$user" = "X" ]; then
  user=jriddell
fi
origdir=`pwd`
udir=umbrello-$version
cd /tmp
svn co -N svn+ssh://${user}@svn.kde.org:/home/kde/branches/KDE/3.5/kdesdk
cd kdesdk
svn co svn+ssh://${user}@svn.kde.org:/home/kde/branches/KDE/3.5/kdesdk/scripts
svn co svn+ssh://${user}@svn.kde.org:/home/kde/branches/KDE/3.5/kdesdk/umbrello $udir
svn co svn+ssh://${user}@svn.kde.org:/home/kde/branches/KDE/3.5/kde-common/admin $udir/admin
svn co -N svn+ssh://${user}@svn.kde.org:/home/kde/branches/KDE/3.5/kdesdk/doc $udir/doc
svn co    svn+ssh://${user}@svn.kde.org:/home/kde/branches/KDE/3.5/kdesdk/doc/umbrello $udir/doc/umbrello
find . -type d -a -name .svn -exec /bin/rm -rf {} \;
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
tar cfvj $tarfile $udir
mv $tarfile $origdir/
cd $origdir
# rm -rf /tmp/kdesdk

echo upload $tarfile to upload.sf.net
echo wput $tarfile ftp://upload.sf.net/incoming/
echo update uml.sf.net including uploading ChangeLog
echo advertise on freshmeat and kde-apps

