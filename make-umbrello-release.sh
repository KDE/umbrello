#!/bin/bash
export CVSROOT=:ext:jriddell@cvs.kde.org:/home/kde
export KDETAG=KDE_3_4_0_ALPHA_1
export UMBRELLO_VERSION=1.4.0alpha1
cvs export -l -r $KDETAG kdesdk
cvs export -l -r $KDETAG kdesdk/doc
cvs export -r $KDETAG kdesdk/doc/umbrello 
cvs export -r $KDETAG kdesdk/umbrello 

mv kdesdk umbrello-${UMBRELLO_VERSION}
cd umbrello-${UMBRELLO_VERSION}
cp -r ~/devel/kdesdk/admin .
make -f Makefile.cvs

cp umbrello/README .
cp umbrello/AUTHORS .
cp umbrello/INSTALL .
cp umbrello/TODO .
#KDE 3.4 cp umbrello/THANKS .
rm kdesdk.lsm
cp umbrello/uml.lsm .
cp umbrello/ChangeLog .

cd ..
tar cfvj umbrello-${UMBRELLO_VERSION}.tar.bz2 umbrello-${UMBRELLO_VERSION}

#test
rm -rf umbrello-${UMBRELLO_VERSION}
tar xfvj umbrello-${UMBRELLO_VERSION}.tar.bz2
cd umbrello-${UMBRELLO_VERSION}
./configure --prefix=/usr
make
cd umbrello/umbrello
./umbrello

cd ../../..
echo upload to upload.sf.net
echo wput umbrello-${UMBRELLO_VERSION}.tar.bz2 ftp://upload.sf.net/incoming/
echo update uml.sf.net including uploading ChangeLog
echo advertise on freshmeat and kde-apps
