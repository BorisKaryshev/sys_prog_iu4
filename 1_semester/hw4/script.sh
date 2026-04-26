#!/bin/bash
set -eou pipefail
set -x

KERNEL_VERSION="6.19.8"

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

    apt update
    apt install -y $packages
    # sudo apt update
    # sudo apt install -y $packages
}

install_dependencies

working_dir="$(pwd)/kernel" #$(mktemp -d)
mkdir -p $working_dir
rm -rf $working_dir/*
cd $working_dir

wget "https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-$KERNEL_VERSION.tar.xz"
tar xf "linux-$KERNEL_VERSION.tar.xz" && mv "linux-$KERNEL_VERSION" linux
cd linux
cp -v /boot/config-$(uname -r) .config

yes "" | make oldconfig || true
scripts/config --disable SYSTEM_TRUSTED_KEYS
scripts/config --disable SYSTEM_REVOCATION_KEYS
yes "" | make oldconfig || true

make -j $(nproc)

exit 0
sudo make modules_install
sudo make install

sudo update-initramfs -c -k "$KERNEL_VERSION"
sudo update-grub

sudo reboot
