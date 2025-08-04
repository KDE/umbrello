#!/bin/bash

# SPDX-FileCopyrightText: 2015-2016 Collabora Ltd.
# SPDX-FileCopyrightText: 2020-2024 Ralf Habacker ralf.habacker @freenet.de
#
# SPDX-License-Identifier: MIT

# add timestamps
export PS4='[$(date "+%T.%3N")]'" $PS4"

set -euo pipefail
set -x

# ci_distro:
# OS distribution in which we are testing
# Typical values: auto opensuse ubuntu
: "${ci_distro:=auto}"

# ci_host:
# the host to build for native, mingw32, mingw64
: "${ci_host:=native}"

# ci_distro_variant:
# Typical values: leap tumbleweed
: "${ci_distro_variant:=leap}"

# ci_variant:
# One of kf5, kf4
: "${ci_variant:=kf5}"

# ci_webserver:
# if yes, install simple webserver
: "${ci_webserver:=yes}"

# setup install command; use sudo outside of docker
# found on https://stackoverflow.com/questions/23513045
case $(cat /proc/1/sched  | head -n 1 | cut -d' ' -f1) in
  systemd|init)
    sudo=sudo
    ci_in_docker="no"
    ;;
  *)
    ci_in_docker="yes"
    sudo=
    ;;
esac

zypper="$sudo /usr/bin/zypper --non-interactive"

install=
source_install=

if [ "$ci_distro" = "auto" ]; then
    ci_distro=$(. /etc/os-release; echo ${ID})
fi

case "$ci_distro" in
    (opensuse*)
        # add required repos
        repo_name=$(. /etc/os-release; echo $PRETTY_NAME | sed 's, ,_,g')
        repos=()
        # install newest cmake
        case "$repo_name" in
            (*Leap*)
                devel_tools_building_repo_name=$(. /etc/os-release; echo $VERSION_ID)
                ;;
            (*Tumbleweed*)
                devel_tools_building_repo_name=openSUSE_Factory
                ;;
        esac
        repos=(
            "${repos[@]}"
            https://download.opensuse.org/repositories/devel:/tools:/building/$devel_tools_building_repo_name/devel:tools:building.repo
        )
        mjv=
        kf=
        qt=
        case "$ci_variant-$ci_host" in
            (*-mingw*)
                bits=$(echo $ci_host | sed 's,mingw,,g')
                repos=(
                    "${repos[@]}"
                    https://download.opensuse.org/repositories/windows:/mingw:/win${bits}/${repo_name}/windows:mingw:win${bits}.repo
                    https://download.opensuse.org/repositories/windows:/mingw/${repo_name}/windows:mingw.repo
                    https://download.opensuse.org/repositories/security:/tls/openSUSE_Tumbleweed/security:tls.repo
                )
                ;;
            (kf6-native)
                repos=(
                    "${repos[@]}"
                    https://download.opensuse.org/repositories/home:/rhabacker:/branches:/windows:/mingw:/staging/${repo_name}/home:rhabacker:branches:windows:mingw:staging.repo
                )
                kf=KF6
                qt=Qt6
                mjv=6
                ;;
            (kf5-native)
                repos=(
                    "${repos[@]}"
                    https://download.opensuse.org/repositories/home:/rhabacker:/branches:/windows:/mingw:/staging/${repo_name}/home:rhabacker:branches:windows:mingw:staging.repo
                )
                kf=KF5
                qt=Qt5
                mjv=5
                ;;

            (kf4-native)
                repos=(
                    "${repos[@]}"
                    https://download.opensuse.org/repositories/windows:/mingw/${repo_name}/windows:mingw.repo
                )
                ;;
        esac

        # setup packages
        packages=(
            AppStream
            ccache
            cmake
            gcc-c++
        )
        source_packages=()

        # misc packages
        packages=(
           "${packages[@]}"
            gawk
            gettext-runtime
            # for catching segfaults
            gdb
            # prevents crashing of mingwxx-windres (https://bugzilla.opensuse.org/show_bug.cgi?id=1198923)
            glibc-locale-base
            # xvfb-run does not have added all required tools
            openbox
            procps # ps
            psmisc # killall
            shadow # useradd
            sharutils # uuencode
            sudo # sudoers
            xvfb-run
            which
            xauth
            xterm
        )

        if [ "$ci_webserver" = "yes" ]; then
            packages=(
            "${packages[@]}"
                php8-cli # webserver
            )
        fi

        # for screenshots
        packages=(
           "${packages[@]}"
            xwd ImageMagick
        )
        
        # common development packages
        case "$ci_variant-$ci_host" in
            (*-native)
                packages=(
                "${packages[@]}"
                    "cmake(${kf}Archive)"
                    "cmake(${kf}Completion)"
                    "cmake(${kf}Config)"
                    "cmake(${kf}CoreAddons)"
                    "cmake(${kf}Crash)"
                    "cmake(${kf}DocTools)"
                    "cmake(${kf}I18n)"
                    "cmake(${kf}IconThemes)"
                    "cmake(${kf}ItemModels)"
                    "cmake(${kf}KCMUtils)"
                    "cmake(${kf}KIO)"
                    "cmake(${kf}TextEditor)"
                    "cmake(${kf}ThreadWeaver)"
                    "cmake(${kf}WidgetsAddons)"
                    "cmake(${kf}WindowSystem)"
                    "cmake(${kf}XmlGui)"
                    "cmake(${qt}Core)"
                    "cmake(${qt}PrintSupport)"
                    "cmake(${qt}Svg)"
                    "cmake(${qt}Test)"
                    "cmake(${qt}Widgets)"
                    "cmake(${qt}Xml)"
                    doxygen
                    libxml2-devel
                    libxslt-devel
                    kdevelop5-pg-qt
                    kdevplatform-devel
                )
            ;;
        esac

        case "$ci_variant-$ci_host" in
            (kf6*-native)
                source_packages=(
                    "${source_packages[@]}"
                )
                packages=(
                    "${packages[@]}"
                    #"cmake(${qt}Concurrent)"
                    #"cmake(${qt}Core5Compat)"
                    #"cmake(${kf}ConfigWidgets)"
                    #"cmake(${kf}Crash)"
                    #"cmake(${kf}Service)"
                    "kf6-extra-cmake-modules"
                )
                ;;

            (kf5*-native)
                source_packages=(
                    "${source_packages[@]}"
                )
                packages=(
                    "${packages[@]}"
                    extra-cmake-modules
                )
                ;;
            (kf5*-mingw*)
                prefix=${ci_host}
                packages=(
                    "${packages[@]}"
                    "$prefix-extra-cmake-modules"
                    "$prefix(cmake:${kf}Archive)"
                    "$prefix(cmake:${kf}Completion)"
                    "$prefix(cmake:${kf}Config)"
                    "$prefix(cmake:${kf}CoreAddons)"
                    "$prefix(cmake:${kf}Crash)"
                    "$prefix(cmake:${kf}DocTools)"
                    "$prefix(cmake:${kf}I18n)"
                    "$prefix(cmake:${kf}IconThemes)"
                    "$prefix(cmake:${kf}KIO)"
                    "$prefix(cmake:${kf}TextEditor)"
                    "$prefix(cmake:${kf}ThreadWeaver)"
                    "$prefix(cmake:${kf}WidgetsAddons)"
                    "$prefix(cmake:${kf}WindowSystem)"
                    "$prefix(cmake:${kf}XmlGui)"
                    "$prefix(cmake:${qt}Core)"
                    "$prefix(cmake:${qt}Gui)"
                    "$prefix(cmake:${qt}PrintSupport)"
                    "$prefix(cmake:${qt}Svg)"
                    "$prefix(cmake:${qt}Test)"
                    "$prefix(cmake:${qt}Widgets)"
                    "$prefix(cmake:${qt}Xml)"
                    wine
                )
                ;;
            (kf4-native)
                # for libQtWebKit-devel
                packages=(
                    "${packages[@]}"
                    extra-cmake-modules
                    libkde4-devel
                    libQtWebKit-devel
                    kdebase4-runtime
                )
                ;;
            (kf4-mingw*)
                prefix=${ci_host}
                packages=(
                    "${packages[@]}"
                    ${prefix}-extra-cmake-modules
                    ${prefix}-libkf4-devel
                    wine
                )
                ;;
            (*)
                echo "unsupported combination '$ci_variant=${ci_variant} ci_host=${ci_host}'"
                exit 1
                ;;
        esac

        # add repos
        for r in ${repos[@]}; do
            $zypper ar --no-refresh --no-check --no-gpgcheck $r || true
        done

        # refresh repos to prevent outdated meta data
        $zypper ref

        if test -v "source_packages"; then
            # enable source repo
            $zypper modifyrepo --enable repo-source
            # install source dependencies
            $zypper source-install -d "${source_packages[@]}"
        fi

        # install remaining packages
        $zypper install "${packages[@]}"
        ;;
esac

# Add the user that we will use to do the build inside the
# Docker container, and let them use sudo
if [ "$ci_in_docker" = "yes" ] && [ -z `getent passwd | grep ^user` ]; then
    useradd -m user
    passwd -ud user
    echo "user ALL=(ALL) NOPASSWD: ALL" > /etc/sudoers.d/nopasswd
    chmod 0440 /etc/sudoers.d/nopasswd
fi
