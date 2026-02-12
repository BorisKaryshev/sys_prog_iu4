#!/bin/bash

set -eou pipefail
# set -x

# Function to display usage
usage() {
    echo "Usage: $0 <config_path> [Options...]"
    echo "Options:"
    echo "  -a, --algorithm <name>    Sets compression algorithm for stored archive"
    echo "                            Supported algorithms are: gzip, zstd, bzip2"
    echo "                            Default: gzip"
    exit 1
}

# Parse command line arguments
if [[ $# -lt 1 ]]; then
    usage
fi

CONFIG_PATH="$1"
shift

COMPRESSION_ALGORITHM="gzip"

# Parse options
while [[ $# -gt 0 ]]; do
    case $1 in
        -a|--algorithm)
            if [[ "$2" =~ ^(gzip|zstd|bzip2)$ ]]; then
                COMPRESSION_ALGORITHM="$2"
            else
                echo "ERROR: Unsupported compression algorithm"
                usage
            fi
            shift 2
            ;;
        -h|--help)
            usage
            ;;
        *)
            echo "Error: Unknown option $1" >&2
            usage
            ;;
    esac
done

target_src="$(cat $CONFIG_PATH | jq '.target_src.[]' -r | xargs -I{} realpath {})"
target_name=$(cat $CONFIG_PATH | jq '.archive_stored_name // "log_rotator"' -r)
target_dst="$(cat $CONFIG_PATH | jq '.target_dst' -r | xargs realpath)/$(date +%Y-%m-%d_%H-%M-%S)_$target_name"
cleanup_policy=$(cat $CONFIG_PATH | jq '.cleanup_policy // "keep_newest"' -r)
n_to_keep=$(cat $CONFIG_PATH | jq '.n_to_keep // 10' -r)

mkdir -p $target_dst
cp $target_src $target_dst

cd $(dirname target_dst)
tar -cf $target_dst.tar $(find $target_dst/* | xargs -I {} realpath --relative-to $(pwd) {})
cd - &> /dev/null

$COMPRESSION_ALGORITHM "$target_dst.tar"
rm -rf $target_dst $target_dst.tar

sort_flag=""
if [[ $cleanup_policy == "keep_oldest" ]]; then
    sort_flag="-r"
fi

ls $(dirname $target_dst) | sort $sort_flag | head -n -$n_to_keep | xargs -I{} rm -rf $(dirname $target_dst)/{}

rm -rf $target_src
touch $target_src
