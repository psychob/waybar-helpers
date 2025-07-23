# Maintainer: Andrzej Budzanowski <kontakt@andrzej.budzanowski.pl>
pkgname=nwc-waybar-helpers
pkgver=0.1.0
pkgrel=1
pkgdesc="Waybar helper utilities"
arch=('x86_64')
url="https://github.com/psychob/waybar-helpers"  # Add your repository URL if desired
license=('AGPL-3+')  # Change to your actual license
depends=('systemd-libs' 'boost-libs')
makedepends=('cmake' 'gcc' 'pkgconf' 'boost')
source=("$pkgname"::"git+file://$startdir")
sha256sums=('SKIP')

build() {
    cd "$srcdir/$pkgname"

    # Create build directory
    mkdir -p build
    cd build

    # Configure with CMake
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_CXX_STANDARD=23

    # Build
    make
}

package() {
    cd "$srcdir/build"

    # Install using CMake
    make DESTDIR="$pkgdir" install
}