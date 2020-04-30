#!/bin/sh

zyp="zypper --non-interactive"

case "$ci_variant" in
    (remote)
        # trigger obs windows build
        token=CBbFTso398hhg4WHESdUPE1n
        curl "http://173.212.229.51/cgi-bin/obs-run-build?repo=${CI_PROJECT_URL}.git&revision=$CI_COMMIT_SHORT_SHA&job=$CI_JOB_ID&apitoken=$token"
        ;;

    (local-kf5)
        # local build
        $zyp install cmake
        $zyp si umbrello
        mkdir -p build && cd build
        CXXFLAGS=-Wno-suggest-override cmake ..
        make -j5
        ctest -VV || true
        make install DESTDIR=$PWD/tmp
        ;;

    (local-mingw32)
        $zyp addrepo --refresh --no-gpgcheck \
            https://download.opensuse.org/repositories/home:/rhabacker:/branches:/windows:/mingw:/win32/openSUSE_Leap_15.1 \
            home:rhabacker:branches:windows:mingw:win32
        #$zyp addrepo --refresh --no-gpgcheck \
        #    https://download.opensuse.org/repositories/home:/rhabacker:/branches:/windows:/mingw:/openSUSE_Leap_15.1 \
        #    home:rhabacker:branches:windows:mingw
        $zyp install \
            cmake \
            extra-cmake-modules \
            make \
            mingw32-cross-binutils \
            mingw32-cross-gcc-c++ \
            mingw32-cross-kde4-tools \
            mingw32-cross-qmake \
            mingw32-kdevelop4-pg-qt \
            mingw32-libqt4-devel \
            mingw32-libqt4-tools \
            mingw32-libqt4-tools-devel \
            mingw32-libkde4-devel \
            mingw32-libkdevplatform4-devel \
            mingw32-libxml2-devel \
            mingw32-libxslt-devel \
            mingw32-oxygen-icon-theme \
            gettext-tools
            # kdevelop4-pg-qt
        mkdir -p build && cd build
        cmake \
            -DCMAKE_SYSTEM_NAME=Windows \
            -DCMAKE_VERBOSE_MAKEFILE=ON \
            -DCMAKE_BUILD_TYPE=RelWithDebInfo \
            -DCMAKE_INSTALL_PREFIX:PATH=/usr/i686-w64-mingw32/sys-root/mingw \
            -DCMAKE_INSTALL_LIBDIR:PATH=/usr/i686-w64-mingw32/sys-root/mingw/lib \
            -DBIN_INSTALL_DIR=/usr/i686-w64-mingw32/sys-root/mingw/bin \
            -DINCLUDE_INSTALL_DIR:PATH=/usr/i686-w64-mingw32/sys-root/mingw/include \
            -DLIB_INSTALL_DIR:PATH=/usr/i686-w64-mingw32/sys-root/mingw/lib \
            -DSHARE_INSTALL_DIR:PATH=/usr/i686-w64-mingw32/sys-root/mingw/share \
            -DSYSCONF_INSTALL_DIR:PATH=/usr/i686-w64-mingw32/sys-root/mingw/etc \
            -DSHARE_INSTALL_PREFIX:PATH=/usr/i686-w64-mingw32/sys-root/mingw/share \
            -DBUILD_SHARED_LIBS:BOOL=ON \
            -DCMAKE_C_COMPILER=/usr/bin/i686-w64-mingw32-gcc \
            -DCMAKE_CXX_COMPILER=/usr/bin/i686-w64-mingw32-g++ \
            -DCMAKE_RC_COMPILER=/usr/bin/i686-w64-mingw32-windres \
            -DCMAKE_FIND_ROOT_PATH=/usr/i686-w64-mingw32/sys-root/mingw \
            -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=ONLY \
            -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=ONLY \
            -DCMAKE_FIND_ROOT_PATH_MODE_PROGRAM=NEVER \
            -DQMAKESPEC=win32-g++-cross \
            -DQT_MKSPECS_DIR:PATH=/usr/i686-w64-mingw32/sys-root/mingw/share/qt4/mkspecs \
            -DQT_QT_INCLUDE_DIR=/usr/i686-w64-mingw32/sys-root/mingw/include \
            -DQT_PLUGINS_DIR=/usr/i686-w64-mingw32/sys-root/mingw/lib/qt4/plugins \
            -DQT_QMAKE_EXECUTABLE:FILEPATH=/usr/bin/i686-w64-mingw32-qmake \
            -DQT_MOC_EXECUTABLE:FILEPATH=/usr/bin/i686-w64-mingw32-moc \
            -DQT_RCC_EXECUTABLE:FILEPATH=/usr/bin/i686-w64-mingw32-rcc \
            -DQT_UIC_EXECUTABLE:FILEPATH=/usr/bin/i686-w64-mingw32-uic \
            -DQT_LUPDATE_EXECUTABLE:FILEPATH=/usr/bin/i686-w64-mingw32-lupdate \
            -DQT_LRELEASE_EXECUTABLE:FILEPATH=/usr/bin/i686-w64-mingw32-lrelease \
            -DQT_DBUSXML2CPP_EXECUTABLE:FILEPATH=/usr/bin/qdbusxml2cpp \
            -DQT_DBUSCPP2XML_EXECUTABLE:FILEPATH=/usr/bin/qdbuscpp2xml \
            -DAutomoc4_DIR=/usr/lib64/automoc4/  \
            -DCMAKE_BUILD_TYPE=RelWithDebInfo \
            -DBUILD_TESTING=OFF \
            -DKDE4_INCLUDE_DIR:PATH=/usr/i686-w64-mingw32/sys-root/mingw/include/kde4 \
            -DKDE4_DATA_DIR=/usr/i686-w64-mingw32/sys-root/mingw/share/apps \
            -DQT_INSTALL_DIR=/usr/i686-w64-mingw32/sys-root/mingw \
            -DKDE4_INSTALL_DIR=/usr/i686-w64-mingw32/sys-root/mingw \
            -DAUTOMOC4_EXECUTABLE=/usr/bin/automoc4 \
            -DAUTOMOC4_VERSION=0.9.88 \
            -DDOCBOOKL10NHELPER_EXECUTABLE=/usr/bin/i686-w64-mingw32-docbookl10nhelper \
            -DICEMAKER_EXECUTABLE=/usr/bin/i686-w64-mingw32-icemaker \
            -DKDE4_KCFGC_EXECUTABLE=/usr/bin/i686-w64-mingw32-kconfig_compiler \
            -DKDE4_KDECONFIG_EXECUTABLE=/usr/bin/i686-w64-mingw32-kde4-config \
            -DKDE4_MEINPROC_EXECUTABLE=/usr/bin/i686-w64-mingw32-meinproc4 \
            -DKDE4_MAKEKDEWIDGETS_EXECUTABLE=/usr/bin/i686-w64-mingw32-makekdewidgets \
            -DPNG2ICO_EXECUTABLE=/usr/bin/i686-w64-mingw32-png2ico  \
            '-DCMAKE_C_FLAGS=-O2 -g -pipe -Wall -fexceptions --param=ssp-buffer-size=4 -mms-bitfields -DNDEBUG -fno-stack-protector -Wno-expansion-to-defined'  \
            '-DCMAKE_CXX_FLAGS=-O2 -g -pipe -Wall -fexceptions --param=ssp-buffer-size=4 -mms-bitfields -DNDEBUG -fno-stack-protector -Wno-expansion-to-defined'  \
            '-DCMAKE_EXE_LINKER_FLAGS=-Wl,--exclude-libs=libintl.a -Wl,--exclude-libs=libiconv.a -Wl,--no-keep-memory -fstack-protector -Wl,--as-needed -Wl,--no-undefined -Wl,-Bsymbolic-functions'  \
            '-DCMAKE_MODULE_LINKER_FLAGS=-Wl,--exclude-libs=libintl.a -Wl,--exclude-libs=libiconv.a -Wl,--no-keep-memory -fstack-protector -Wl,--as-needed -Wl,--no-undefined -Wl,-Bsymbolic-functions'  \
            '-DCMAKE_SHARED_LINKER_FLAGS=-Wl,--exclude-libs=libintl.a -Wl,--exclude-libs=libiconv.a -Wl,--no-keep-memory -fstack-protector -Wl,--as-needed -Wl,--no-undefined -Wl,-Bsymbolic-functions'  \
            ..
        make -j5
        ctest -VV || true
        make install DESTDIR=$PWD/tmp
        ;;

    (check)
        uname -a
        cat /etc/os-release
        set
        zypper lr --url
        ifconfig
        route
        ;;
esac
