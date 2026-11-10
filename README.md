# 🔐 sshole lab (CVE-2026-123158)

🐳 A Docker lab that reproduces **"sshole" (CVE-2026-123158)**, an authentication bypass in OpenSSH `sshd` when it is built against **musl libc** and **without PAM**. An account whose `/etc/shadow` password field is `*` (for example, a key-only account) accepts any password between 257 and 1024 bytes.

📚 **References:** [sshhole.com](https://sshhole.com) (the discoverer's write-up) and [github.com/oz9un/sshole](https://github.com/oz9un/sshole) (the original PoC).

> ⚠️ For research and defense on systems you own. The ports are meant for `localhost` only. Do not expose this lab to a network.

## 🐛 Vulnerability summary

The bug comes from two behaviors that are each reasonable on their own:

1. **🔑 OpenSSH compares crypt output with the stored field.** For an account with `*` as its password field (locked or key-only), `sshd` has no real hash to compare against. Since CVE-2016-6210, it substitutes a salt from another account so that unknown and locked accounts take similar time to reject. It then compares the `crypt()` output with the stored `*`.
2. **⚙️ musl's `crypt()` returns `*` on failure.** The SHA-512 crypt implementation in musl rejects keys longer than 256 bytes and returns `*` instead of NULL. A password of 257 to 1024 bytes therefore produces `*`, which matches the locked account's stored `*`.

Together, a password of 257 to 1024 bytes logs into any account whose field is `*` on a musl system where `sshd` checks passwords itself (no PAM). Fixed in OpenSSH 10.6p2; Alpine backported the fix to its packages (see "Why OpenSSH is built from source").

## 🧪 What it demonstrates

| Container | libc | sshd | Target account `alice` | Expected result with a 300-byte password |
|---|---|---|---|---|
| `sshole-alpine-vuln` | musl (Alpine 3.18) | OpenSSH 9.3p2 built without PAM | field `*` | **✅ Login accepted (vulnerable)** |
| `sshole-debian-control` | glibc (Debian 12) | Debian's OpenSSH with PAM | field `*` | **❌ Rejected (control)** |

With a 256-byte password, the vulnerable container also rejects the login. musl's limit (256 bytes for sha512crypt) is not reached, so `crypt()` returns a real hash that does not match.

## 📋 Requirements

- 🐳 Docker with Compose v2 (`docker compose`).
- 🌐 Internet access during the build: the build downloads packages from Alpine and Debian and the OpenSSH tag from GitHub.
- ⚠️ On hosts with an IPv6 link-local DNS server, the build can fail to resolve names. The compose file uses `network: host` for the builds to avoid this.

## 📁 Layout

```text
sshole-lab/
├── docker-compose.yml        # Services: alpine-vuln, debian-control, attacker (profile "attack")
├── run_exploit.sh            # Starts the lab and runs the three test attempts
├── check.sh                  # Official exposure checker from github.com/oz9un/sshole (run inside containers)
├── alpine-vuln/
│   └── Dockerfile            # Alpine 3.18 + OpenSSH 9.3p2 built from upstream, without PAM
├── debian-control/
│   └── Dockerfile            # Debian 12-slim + openssh-server (glibc, PAM)
├── attacker/
│   ├── Dockerfile            # Alpine + python3 + paramiko
│   ├── attack.py             # PoC: one connection, two password attempts
│   ├── crypt_spy.c           # Diagnostics: LD_PRELOAD shim that logs crypt()/strcmp() calls
│   └── test_picksalt.c       # Diagnostics: reproduces pick_salt() + crypt() outside sshd
```

🔎 The two `.c` files in `attacker/` are diagnostic tools I used to investigate the internal behavior. They are not part of the reproduction.

## 🔍 Checking exposure with check.sh

`check.sh` is the official checker from the sshole repository. It reports whether the local `sshd` meets the exposure conditions, but it does not attempt a login.

```bash
docker cp check.sh sshole-alpine-vuln:/tmp/check.sh
docker exec sshole-alpine-vuln sh /tmp/check.sh      # EXPOSED, exit 0
docker cp check.sh sshole-debian-control:/tmp/check.sh
docker exec sshole-debian-control sh /tmp/check.sh   # not exposed (not musl), exit 1
```

## 🚀 How to run it

From `sshole-lab/`:

```bash
# Start both servers (build included)
docker compose up -d --build

# Run the full reproduction
bash run_exploit.sh
```

### 🎯 Manual attack

Or attack manually once the containers are running:

```bash
# Syntax: attack.py HOST PORT USER LENGTH
docker compose --profile attack run --rm attacker sshole-alpine-vuln 22 alice 300
```

Inside the Docker network the port is always `22`. The host ports are `2222` (Alpine) and `2223` (Debian), if you want to connect from outside Docker with `ssh -p 2222 alice@localhost`.

### 📊 Expected output of `run_exploit.sh`

The script runs two steps: the exposure check with `check.sh`, then the login attempts.

```text
== check.sh : sshole-alpine-vuln ==
