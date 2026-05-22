pkgname=bearwave-git
pkgver=1.0.1
pkgrel=1
pkgdesc="A KDE-focused desktop internet radio app"
arch=('x86_64')
url="https://github.com/bearwave/bearwave" # Replace with actual URL if different
license=('MIT')
depends=('qt6-base' 'qt6-declarative' 'kirigami' 'ki18n' 'kcoreaddons' 'phonon-qt6')
makedepends=('cmake' 'extra-cmake-modules' 'git')
provides=('bearwave')
conflicts=('bearwave')
source=("git+file://${PWD}")
md5sums=('SKIP')

pkgver() {
  cd "${srcdir}/${pkgname%-git}"
  printf "1.0.1.r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
  cmake -B build -S "${pkgname%-git}" \
    -DCMAKE_BUILD_TYPE='Release' \
    -DCMAKE_INSTALL_PREFIX='/usr'
  cmake --build build -j"$(nproc)"
}

package() {
  DESTDIR="$pkgdir" cmake --install build
  install -Dm644 "${pkgname%-git}/LICENSE" "$pkgdir/usr/share/licenses/$pkgname/LICENSE"
}
