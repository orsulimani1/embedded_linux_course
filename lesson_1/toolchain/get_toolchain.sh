TOOLCHAIN_URL="https://snapshots.linaro.org/gnu-toolchain/14.0-2023.06-1/arm-linux-gnueabihf/gcc-linaro-14.0.0-2023.06-x86_64_arm-linux-gnueabihf.tar.xz"
DEST_DIR="gcc-linaro-14.0.0-2023.06-x86_64_arm-linux-gnueabihf"

# Extract the filename from the URL (for convenience).
TARBALL="$(basename "$TOOLCHAIN_URL")"

echo "Creating destination directory: $DEST_DIR"
mkdir -p "$DEST_DIR"

echo "Downloading toolchain tarball: $TARBALL"
wget -O "$TARBALL" "$TOOLCHAIN_URL"

echo "Extracting $TARBALL into $DEST_DIR"
tar -xf "$TARBALL" -C "$DEST_DIR" --strip-components=1


# Create symbolic link for gcc
echo "Creating symbolic link..."
cd gcc-linaro-14.0.0-2023.06-x86_64_arm-linux-gnueabihf/bin

ln -s arm-linux-gnueabihf-gcc-14.0.0 arm-linux-gnueabihf-gcc


rm -f gcc-linaro-14.0.0-2023.06-x86_64_arm-linux-gnueabihf.tar.xz