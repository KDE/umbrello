#!/bin/sh

#------------------------
# dump env
set

case "$ci_variant" in
    (remote)
        # trigger obs windows build
        token=CBbFTso398hhg4WHESdUPE1n
        curl "http://173.212.229.51/cgi-bin/run-obs-build?repo=${CI_PROJECT_URL}.git&revision=$CI_COMMIT_SHORT_SHA&job=$CI_JOB_ID&apitoken=$token"
        ;;

    (local)
        # local build
        zypper --non-interactive si umbrello
        mkdir -p build && cd build
        cmake ..
        make -j5
        ;;
esac
