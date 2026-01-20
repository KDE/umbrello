#!/bin/sh
#
# @author Ralf Habacker <ralf.habacker@freenet.de>
#

# run as
#  ci_distro=... ci_parallel=... ci_variant=... sudo -E tools/ci-run-docker-image.sh
# or
#  sudo ci_distro=... ci_parallel=... ci_variant=... tools/ci-run-docker-image.sh

# the docker image to use
: "${ci_image:=opensuse/leap:15.6}"

# the distribution to use
: "${ci_distro:=}"

# enter source dir
# cd ~/src/umbrello

case "$ci_distro" in
    (opensuse-leap|opensuse-leap-15.6)
        ci_image=opensuse/leap:15.6
        ;;
    (opensuse-leap-16)
        ci_image=opensuse/leap:16.0
        ;;
    (opensuse-tumbleweed)
        ci_image=opensuse/tumbleweed
        ;;
    (ubuntu)
        ci_image=ubuntu:20.10
        ;;
esac

#inside docker run
cat << EOF
#inside docker run the following commands

cd /mnt

tools/ci-install.sh
tools/ci-build.sh
EOF

options=
shopts="export ci_parallel=$ci_parallel; export ci_distro=$ci_distro; export ci_variant=$ci_variant;"
if [ "$1" == "--use-host-display" ]; then
    options="-v $HOME/.Xauthority:/root/.Xauthority:rw --env=DISPLAY --net=host"
    shopts+=" export DISPLAY=$DISPLAY;"
fi

sudo docker pull $ci_image
sudo docker run \
    -v $PWD:/mnt \
    $options \
    -it $ci_image \
    /bin/bash -c "cd /mnt; $shopts tools/ci-install.sh; tools/ci-build.sh; bash"

