# Debian (glibc) CONTROL image: same account layout, NOT vulnerable.
# glibc's crypt() does not have musl's "return * on oversized input" behavior,
# and Debian's sshd goes through PAM, which never calls crypt() for a "*" field.
# Used to show the exploit does NOT work here.
FROM debian:12-slim

RUN apt-get update \
 && apt-get install -y --no-install-recommends openssh-server \
 && rm -rf /var/lib/apt/lists/* \
 && mkdir -p /run/sshd /etc/ssh/sshd_config.d \
 && echo "PermitRootLogin yes" > /etc/ssh/sshd_config.d/00-lab.conf \
 && useradd -m -s /bin/sh alice \
 && useradd -m -s /bin/sh bob \
 && echo 'bob:BobPass-123' | chpasswd \
 && sed -i 's/^alice:[^:]*:/alice:*:/' /etc/shadow

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D", "-e"]
