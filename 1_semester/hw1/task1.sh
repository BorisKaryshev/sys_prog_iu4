#!/bin/bash

set -eou pipefail
set -x

echo $1

last "$1" -n 3
lastlog -u "$1"
