# Throwaway attacker image: python3 + paramiko + attack.py (same technique as
# the official PoC at github.com/oz9un/sshole/poc/attack.py).
FROM alpine:3.18
RUN apk add --no-cache python3 py3-paramiko
COPY attack.py /attack.py
ENTRYPOINT ["python3", "/attack.py"]
