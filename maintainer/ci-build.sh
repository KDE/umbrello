#!/bin/sh

# $1 - repo url
# $2 - git branch

#------------------------
# trigger obs windows build 

if test "$1" == "master"; then
    token=CBbFTso398hhg4WHESdUPE1n
elif test "$1" == "release/19.12"; then
    token=CoFGozRfz3m54BLePBHF1bHT
fi

# perform api call
curl -H "Authorization: Token $token" -X POST https://api.opensuse.org/trigger/runservice

#------------------------
# build unix variant

mkdir -p build && cd build
cmake -G Ninja ..
ninja
