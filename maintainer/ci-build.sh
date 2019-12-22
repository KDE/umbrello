#!/bin/sh

# $1 - repo url
# $2 - git branch

#------------------------
# dump env
set

#------------------------
# trigger obs windows build 
token=CBbFTso398hhg4WHESdUPE1n
curl "http://173.212.229.51/cgi-bin/run-obs-build?repo=${CI_PROJECT_URL}.git&revision=$CI_COMMIT_SHORT_SHA&job=$CI_JOB_ID&apitoken=$token"


#------------------------
# build unix variant

mkdir -p build && cd build
cmake -G Ninja ..
ninja
