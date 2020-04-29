#!/bin/sh

case "$ci_variant" in
    (remote)
        # trigger obs windows build
        token=CBbFTso398hhg4WHESdUPE1n
        curl "http://173.212.229.51/cgi-bin/obs-run-build?repo=${CI_PROJECT_URL}.git&revision=$CI_COMMIT_SHORT_SHA&job=$CI_JOB_ID&apitoken=$token"
        ;;

    (local)
        # local build
        zypper --non-interactive install cmake
        zypper --non-interactive si umbrello
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
