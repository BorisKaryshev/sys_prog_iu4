#!/usr/bin/env python3
import sys

PROC_FILE = str(sys.argv[1])


def cmd(line):
    with open(PROC_FILE, "w") as f:
        f.write(line + "\n")
        f.flush()
    with open(PROC_FILE, "r") as f:
        return f.read().strip()


banner = """kernel crypto wallet demo (procfs version)
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
