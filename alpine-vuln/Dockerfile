# Alpine image VULNERABLE to CVE-2026-123158 ("*sshole").
#
# The Alpine package openssh-server 9.3_p2-r4 in this image is built Nov 2026,
# i.e. it already contains Alpine's backported fix, so it cannot reproduce the
# bug. This image therefore compiles upstream OpenSSH 9.3p2 (tag V_9_3_P2,
# unpatched auth-passwd.c / xcrypt.c) against musl, WITHOUT PAM.
#
#   alice -> shadow field "*"   (key-only / "no password" account = the target)
#   bob   -> real $6$ hash      (ordinary user)
#   root  -> real $6$ hash      (pick_salt() reads root's salt for "*" accounts)
FROM alpine:3.18

ARG OPENSSH_TAG=V_9_3_P2
RUN apk add --no-cache --virtual .build-deps \
      build-base autoconf automake linux-headers openssl-dev zlib-dev wget \
 && apk add --no-cache openssl zlib \
 && wget -q -O /tmp/openssh.tar.gz \
      "https://github.com/openssh/openssh-portable/archive/refs/tags/${OPENSSH_TAG}.tar.gz" \
 && mkdir -p /tmp/src && tar -xzf /tmp/openssh.tar.gz -C /tmp/src --strip-components=1 \
 && cd /tmp/src \
 && autoreconf -i >/dev/null 2>&1 \
 && ./configure --prefix=/usr --sysconfdir=/etc/ssh --with-privsep-path=/var/empty \
      --with-privsep-user=sshd --without-pam --without-libedit --without-ldns \
 && make -j"$(nproc)" >/dev/null \
 && make install-nokeys >/dev/null \
 && cd / && rm -rf /tmp/src /tmp/openssh.tar.gz \
 && apk del .build-deps

RUN getent group sshd >/dev/null || addgroup -S sshd \
 && id -u sshd >/dev/null 2>&1 || adduser -S -G sshd -H -h /var/empty -s /sbin/nologin sshd \
 && mkdir -p /var/empty /var/run/sshd \
 && ssh-keygen -A \
 && adduser -D -s /bin/sh alice && adduser -D -s /bin/sh bob \
 && echo 'bob:BobPass-123' | chpasswd \
 && echo 'root:RootRealPass-456' | chpasswd \
 && sed -i 's/^alice:[^:]*:/alice:*:/' /etc/shadow \
 && printf '\nPasswordAuthentication yes\nPermitRootLogin yes\nUsePAM no\n' >> /etc/ssh/sshd_config

EXPOSE 22
CMD ["/usr/sbin/sshd", "-D", "-e"]
