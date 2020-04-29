#!/bin/sh

#------------------------
# dump env
set

#------------------------
# trigger obs windows build 
token=CBbFTso398hhg4WHESdUPE1n
curl "http://173.212.229.51/cgi-bin/run-obs-build?repo=${CI_PROJECT_URL}.git&revision=$CI_COMMIT_SHORT_SHA&job=$CI_JOB_ID&apitoken=$token"


#------------------------
# build unix variant
if test -n "$CI_UNIX"; then
    # local build
    zypper --non-interactive si umbrello
    git clone https://invent.kde.org/kde/umbrello.git
    mkdir -p umbrello-build && cd umbrello-build
    cmake ../umbrello
    make -j5
fi
