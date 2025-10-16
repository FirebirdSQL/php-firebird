# For building linux-x64
FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential autoconf bison re2c wget tar git patchelf libtommath1

# Download php
ARG php_vers="7.4.13 8.0.30 8.1.33 8.2.29 8.3.26 8.4.13 8.5.0RC2"
RUN --mount=type=cache,target=/usr/src \
    cd /usr/src && \
    set -e && \
    for php in $php_vers; do \
        if [ ! -d php-$php ]; then \
            git clone --depth 1 --branch php-$php https://github.com/php/php-src.git php-$php; \
        fi \
    done

# Build php
RUN --mount=type=cache,target=/usr/src \
    for php in $php_vers; do \
        php_short=${php%.*} && \
        cd /usr/src/php-$php && \
        ./buildconf --force && \
        ./configure \
            --prefix=/opt/php-$php_short \
            --disable-all --disable-cgi --enable-cli --build=x86_64-linux-gnu --host=x86_64-linux-gnu && \
        make -j$(nproc) && make install; \
    done

# Download Firebird-5.0.3
ARG firebird=Firebird-5.0.3.1683-0-linux-x64
RUN --mount=type=cache,target=/usr/src \
    cd /usr/src && \
    if [ ! -d $firebird ]; then \
        wget https://github.com/FirebirdSQL/firebird/releases/download/v5.0.3/$firebird.tar.gz; \
    fi && \
    tar xf $firebird.tar.gz && \
    mkdir -p /opt/$firebird && \
    tar xf $firebird/buildroot.tar.gz -C /opt/$firebird

# Clone php-firebird
RUN --mount=type=cache,target=/usr/src \
    cd /usr/src && \
    if [ -d php-firebird ]; then \
        cd php-firebird && git pull; \
    else \
        git clone --depth 1 --branch "master" "https://github.com/FirebirdSQL/php-firebird.git"; \
    fi

# Build php-firebird
RUN --mount=type=cache,target=/usr/src \
    for php in $php_vers; do \
        php_short=${php%.*} && \
        cd /usr/src/php-firebird && \
        (make clean || true) && \
        (/opt/php-$php_short/bin/phpize --clean || true) && \
        /opt/php-$php_short/bin/phpize && \
        ./configure \
            --with-php-config=/opt/php-$php_short/bin/php-config \
            --with-interbase=/opt/$firebird/opt/firebird || (cat config.log && false) && \
        make -j$(nproc) && \
        patchelf --remove-rpath ./modules/interbase.so && \
        cp ./modules/interbase.so /opt/php${php_short}-interbase-5.0.3-linux.so; \
    done

# TODO: git config --global advice.detachedHead false
