#!/bin/bash

set -eou pipefail
# set -x

# Функция для отображения справки
usage() {
    cat << EOF
Usage: $0 <config_path> [Options...]
Options:
  -a, --algorithm <name>    Sets compression algorithm for stored archive
                            Supported algorithms are: gzip, zstd, bzip2
                            Default: gzip
EOF
    exit 1
}

# Функция для проверки зависимостей
check_dependencies() {
    local deps=("jq" "realpath")
    for dep in "${deps[@]}"; do
        if ! command -v "$dep" &> /dev/null; then
            echo "Error: Required dependency '$dep' is not installed" >&2
            exit 1
        fi
    done
}

# Функция для разбора конфигурации
parse_config() {
    local config_path="$1"
    target_src=($(jq '.target_src.[]' -r "$config_path"))
    target_name=$(jq '.archive_stored_name // "log_rotator"' -r "$config_path")
    target_dst=$(realpath "$(jq '.target_dst' -r "$config_path")")
    cleanup_policy=$(jq '.cleanup_policy // "keep_newest"' -r "$config_path")
    n_to_keep=$(jq '.n_to_keep // 10' -r "$config_path")
}

# Основная логика
main() {
    check_dependencies

    if [[ $# -lt 1 ]]; then
        usage
    fi

    local CONFIG_PATH="$1"
    shift
    local COMPRESSION_ALGORITHM="gzip"

    # Парсинг опций
    while [[ $# -gt 0 ]]; do
        case $1 in
            -a|--algorithm)
                case "$2" in
                    gzip|zstd|bzip2)
                        COMPRESSION_ALGORITHM="$2"
                        ;;
                    *)
                        echo "ERROR: Unsupported compression algorithm '$2'" >&2
                        usage
                        ;;
                esac
                shift 2
                ;;
            -h|--help)
                usage
                ;;
            *)
                echo "Error: Unknown option '$1'" >&2
                usage
                ;;
        esac
    done

    # Проверка существования конфигурационного файла
    if [[ ! -f "$CONFIG_PATH" ]]; then
        echo "Error: Config file '$CONFIG_PATH' not found" >&2
        exit 1
    fi

    # Загрузка конфигурации
    parse_config "$CONFIG_PATH"

    # Подготовка путей
    local current_dst="${target_dst}/$(date +%Y-%m-%d_%H-%M-%S)_${target_name}"
    local archive_dir=$(dirname "$current_dst")

    # Создание временной директории
    mkdir -p "$current_dst"

    # Копирование файлов
    for src in "${target_src[@]}"; do
        if [[ -e "$src" ]]; then
            cp "$src" "$current_dst/"
        else
            echo "Warning: Source '$src' does not exist" >&2
        fi
    done

    # Создание архива
    local archive_path="${current_dst}.tar"
    tar -cf "$archive_path" -C "$archive_dir" "$(basename "$current_dst")"

    # Сжатие архива
    case "$COMPRESSION_ALGORITHM" in
        gzip) gzip "$archive_path" ;;
        zstd) zstd "$archive_path" ;;
        bzip2) bzip2 "$archive_path" ;;
    esac

    # Очистка временных файлов
    rm -rf "$current_dst"

    # Управление версиями архивов
    local sort_flag=""
    [[ "$cleanup_policy" == "keep_oldest" ]] && sort_flag="-r"

    find "$archive_dir" -maxdepth 1 -name "*_${target_name}.tar.*" \
        | sort $sort_flag \
        | head -n -"$n_to_keep" \
        | xargs -I{} rm -f {}

    # Очистка исходных файлов
    for src in "${target_src[@]}"; do
        if [[ -e "$src" ]]; then
            rm -rf "$src"
            touch "$src"
        fi
    done
}

main "$@"
