#!/bin/bash

# Configuration
FIREBIRD_INCLUDE_DIR="/opt/firebird/include"
INSTALL_DIR="../ext"
REPO_URL="https://github.com/FirebirdSQL/php-firebird.git"
BRANCH_OR_COMMIT="master" # Set to a specific tag or commit if needed
PHP_VERSIONS=("7.4" "8.0" "8.1" "8.2" "8.3" "8.4") # Adjust as needed
BUILD_DIR="php-firebird-build"

set -e

mkdir -p "$INSTALL_DIR"
rm -rf "$BUILD_DIR"
echo "Cloning repository from $REPO_URL (branch: $BRANCH_OR_COMMIT)..."
git clone --depth 1 --branch "$BRANCH_OR_COMMIT" "$REPO_URL" "$BUILD_DIR"

# Read version from driver
if [[ -f "$BUILD_DIR/php_interbase.h" ]]; then
    DRIVER_VERSION=$(grep -Eo 'PHP_INTERBASE_VERSION\s+"[0-9.]+"' "$BUILD_DIR/php_interbase.h" | grep -Eo '[0-9.]+')
elif [[ -f "$BUILD_DIR/package.xml" ]]; then
    DRIVER_VERSION=$(grep -oPm1 "(?<=<version>)[^<]+" "$BUILD_DIR/package.xml")
else
    DRIVER_VERSION="unknown"
fi

for VERSION in "${PHP_VERSIONS[@]}"; do
    echo "==> Building for PHP $VERSION"

    PHP_BIN="/usr/bin/php$VERSION"
    PHPIZE="/usr/bin/phpize$VERSION"
    PHP_CONFIG="/usr/bin/php-config$VERSION"

    if [[ ! -x "$PHP_BIN" || ! -x "$PHPIZE" || ! -x "$PHP_CONFIG" ]]; then
        echo "--> Installing missing PHP $VERSION packages..."
        sudo apt-get install -y "php$VERSION-dev" "php$VERSION-cli" "php$VERSION-common"
    fi

    cd "$BUILD_DIR"

    echo "--> Cleaning previous build (if any)..."
    make clean || true
    echo "--> Running phpize..."
    "$PHPIZE"
    echo "--> Configuring build..."
    CPPFLAGS="-I$FIREBIRD_INCLUDE_DIR" ./configure --with-php-config="$PHP_CONFIG"
    echo "--> Compiling..."
    make -j"$(nproc)"

    PHP_FULL_VERSION=$("$PHP_BIN" -r 'echo PHP_VERSION;')
    ARCH=$(uname -m)
    OS=$(uname -s | tr '[:upper:]' '[:lower:]')

    OUTPUT_FILE="php_${PHP_FULL_VERSION}-interbase-${DRIVER_VERSION}-${OS}-${ARCH}.so"
    mkdir -p "$INSTALL_DIR"
    echo "--> Copying output to $INSTALL_DIR/$OUTPUT_FILE"
    cp modules/interbase.so "$INSTALL_DIR/$OUTPUT_FILE"

    echo "Build complete for PHP $VERSION: $OUTPUT_FILE"
    cd ..
done

echo "All builds completed. Files are located in: $INSTALL_DIR"

