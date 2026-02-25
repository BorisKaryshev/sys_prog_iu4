#!/bin/bash
set -eou pipefail
set -x

find . -name build -type d | xargs rm -rf
