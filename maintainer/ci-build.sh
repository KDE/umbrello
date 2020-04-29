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
        CXXFLAGS=-Wno-suggest-override cmake ..
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
