#!/bin/bash
set -eou pipefail
set -x

apt update
apt install -y libssl-dev valgrind cmake g++ git libboost-all-dev
