#!/usr/bin/env python3
import sys
import os
import fcntl
import array

DEVICE = "/dev/kwallet"
# TODO: Replace with the actual ioctl command number from your kernel driver
# Example: KWALLET_IOCTL_CMD = 0x12345678
KWALLET_IOCTL_CMD = 0xDEADBEEF
BUFFER_SIZE = 4096

# Open the device once (read/write) and keep the file descriptor
try:
    fd = os.open(DEVICE, os.O_RDWR)
except OSError as e:
    sys.exit(f"Failed to open {DEVICE}: {e}")


def cmd(line: str) -> str:
    """Send a command via ioctl and return the stripped response."""
    # Prepare command string with newline (as original write did)
    cmd_str = line + "\n"
    cmd_bytes = cmd_str.encode()

    # Allocate a mutable buffer, copy command into it
    # Use array of bytes (type code 'b') for maximum compatibility
    buf = array.array("b", [0]) * BUFFER_SIZE
    for i, b in enumerate(cmd_bytes):
        if i < BUFFER_SIZE:
            buf[i] = b

    try:
        # Perform ioctl - all arguments are positional, no keywords
        # The 'mutate_flag' is the 4th positional argument (True means buffer can be modified)
        fcntl.ioctl(fd, KWALLET_IOCTL_CMD, buf, True)
    except OSError as e:
        return f"ioctl error: {e}"

    # Find the null terminator or end of response
    response_bytes = []
    for i in range(BUFFER_SIZE):
        if buf[i] == 0:
            break
        response_bytes.append(buf[i])

    response = bytes(response_bytes).decode().strip()
    return response


banner = """kernel crypto wallet demo
Commands:
  create    <keyname> <privkey>
  read      <keyname>
  readall
  change    <keyname> <oldkey> <newkey>
  delete    <keyname>
  deleteall
  quit
"""

if __name__ == "__main__":
    print(banner)
    while True:
        try:
            raw = input("kwallet > ")
        except KeyboardInterrupt:
            sys.exit(0)
        raw = raw.strip()
        if not raw:
            continue
        if raw.lower() in {"quit", "q"}:
            break
        print(cmd(raw))

    os.close(fd)  # clean up
