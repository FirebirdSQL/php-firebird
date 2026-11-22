# docker build -t fb25 - < fb25.dockerfile
# docker run -d --name fb25 --restart unless-stopped -p 3051:3050 -v /tmp:/tmp fb25

# Firebird 2.5.9 SuperServer on Debian bullseye (glibc 2.31).
# 2.5 crashes at random on glibc >= 2.34, see
# https://firebirdsql.org/en/news/urgent-and-import-message-for-users-of-firebird-2-5-on-linux/
FROM debian:bullseye-slim

ADD https://github.com/FirebirdSQL/firebird/releases/download/R2_5_9/FirebirdSS-2.5.9.27139-0.amd64.tar.gz /tmp/fb.tar.gz

# bullseye is end-of-life and its security repository is gone; main still works.
# uid/gid match the host's firebird user, so database files it creates in a
# bind-mounted /tmp are owned the same way as those of the native servers.
RUN rm -f /etc/apt/sources.list.d/* \
 && echo "deb http://deb.debian.org/debian bullseye main" > /etc/apt/sources.list \
 && apt-get update \
 && apt-get install -y --no-install-recommends libncurses5 libstdc++6 \
 && rm -rf /var/lib/apt/lists/* \
 && groupadd -g 110 firebird && useradd -u 103 -g 110 -M -d /opt/firebird -s /usr/sbin/nologin firebird \
 && tar -xzf /tmp/fb.tar.gz -C /tmp \
 && tar -xzf /tmp/FirebirdSS-*/buildroot.tar.gz -C / \
 && rm -rf /tmp/fb.tar.gz /tmp/FirebirdSS-* \
 && chown -R firebird:firebird /opt/firebird

# Own lock dir: with the host's /tmp mounted, the default /tmp/firebird would be
# shared with the host's 3.0/4.0/5.0 servers
ENV FIREBIRD_LOCK=/var/lib/firebird-lock
RUN install -d -o firebird -g firebird -m 770 /var/lib/firebird-lock

USER firebird
EXPOSE 3050
# fbguard without -daemon stays in the foreground and restarts fbserver if it dies
CMD ["/opt/firebird/bin/fbguard", "-forever"]
