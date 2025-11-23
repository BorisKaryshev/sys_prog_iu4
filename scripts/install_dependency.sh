#!/bin/bash
set -eou pipefail
set -x

apt update
apt install -y valgrind cmake g++ git libboost-all-dev
