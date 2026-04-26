#!/usr/bin/env python3
import sys, os, time

DEV = "/dev/kwallet"


def cmd(line):
    with open(DEV, "w") as f:
        f.write(line + "\n")
        f.flush()
    with open(DEV, "r") as f:
        return f.read().strip()


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
