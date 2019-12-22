#!/bin/sh

# $1 - repo url
# $2 - git branch

#------------------------
# trigger obs windows build 
token=
if test "$2" == "master"; then
    token=CBbFTso398hhg4WHESdUPE1n
elif test "$2" == "release/19.12"; then
    token=CoFGozRfz3m54BLePBHF1bHT
fi

# perform api call
if test -n "$token"; then
    curl -H "Authorization: Token $token" -X POST https://api.opensuse.org/trigger/runservice
fi

#------------------------
# build unix variant

mkdir -p build && cd build
cmake -G Ninja ..
ninja
