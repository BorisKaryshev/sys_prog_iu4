#!/bin/bash 
set -eou pipefail
set -x 

install_dependencies ()
{
    packages=$(
cat <<EOF
        build-essential 
        fakeroot 
        libncurses-dev 
        libncurses5-dev 
        bc 
        bison 
        flex 
        libssl-dev 
        libelf-dev 
        kmod 
        ccache 
        dwarves 
        git 
        rsync 
        gettext 
        libnuma-dev 
        xz-utils 
        gzip 
        bzip2 
        unzip 
        python3 
        python3-dev
EOF
    )

    packages="$(echo $packages | xargs echo)"

    sudo apt update
    sudo apt install -y $packages
}

install_dependencies

KERNEL_VERSION="6.17.8"

working_dir=$(mktemp -d)
cd $working_dir

wget "https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-$KERNEL_VERSION.tar.xz"
tar xf "linux-$KERNEL_VERSION.tar.xz" && mv "linux-$KERNEL_VERSION" linux
cd linux 

yes "" | make oldconfig || true
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
yes "" | make oldconfig || true

make -j $(nproc)
