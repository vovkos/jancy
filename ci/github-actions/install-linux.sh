#...............................................................................
#
#  This file is part of the AXL library.
#
#  AXL is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/axl/license.txt
#
#...............................................................................

LLVM_VERSION_MAJOR=${LLVM_VERSION#llvm-}

# llvm is unpacked from .debs without resolving dependencies:
# llvm-tools depends on python3:<arch> which conflicts with way too much

install_llvm () {
	DEB_ARCH=$1
	LLVM_DIR=$HOME/llvm
	LLVM_DEB_DIR=$HOME/llvm-deb

	mkdir -p $LLVM_DEB_DIR
	pushd $LLVM_DEB_DIR

	# linker-tools carries libLTO.so required by LLVMExports.cmake

	apt-get download \
		"${LLVM_VERSION}-dev:$DEB_ARCH" \
		"${LLVM_VERSION}-linker-tools:$DEB_ARCH"

	# the shared-lib package got t64 suffix in ubuntu-24

	apt-get download libllvm${LLVM_VERSION_MAJOR}t64:$DEB_ARCH || \
	apt-get download libllvm${LLVM_VERSION_MAJOR}:$DEB_ARCH

	popd

	for DEB in $LLVM_DEB_DIR/*.deb; do
		dpkg -x $DEB $LLVM_DIR
	done

	echo "set (LLVM_CMAKE_DIR $LLVM_DIR/usr/lib/$LLVM_VERSION/lib/cmake/llvm)" >> paths.cmake
	echo "set (LLVM_CONFIG_EXE NOTFOUND)" >> paths.cmake
}

sudo apt-get remove -y --purge '^llvm(-[0-9]+)?-dev(:i386)?$'

if [[ $TARGET_CPU != "x86" ]]; then
	sudo apt-get -qq update

	sudo apt-get install -y liblua5.2-dev
	sudo apt-get install -y libpcap-dev
	sudo apt-get install -y libudev-dev
	sudo apt-get install -y libusb-1.0
	sudo apt-get install -y libz-dev

	install_llvm $TARGET_CPU
else
	sudo dpkg --add-architecture i386
	sudo apt-get -qq update

	sudo apt-get install -y liblua5.2-dev:i386
	sudo apt-get install -y libpcap-dev:i386
	sudo apt-get install -y libudev-dev:i386
	sudo apt-get install -y libusb-1.0:i386
	sudo apt-get install -y libssl-dev:i386
	sudo apt-get install -y zlib1g-dev:i386

	# llvm-18's exports add zstd::libzstd_shared and LibEdit::LibEdit

	sudo apt-get install -y libzstd-dev:i386
	sudo apt-get install -y libedit-dev:i386

	install_llvm i386

	# install g++-multilib -- in the end, after i386 packages!

	sudo apt-get install -y g++-multilib
fi

sudo apt-get install -y p7zip-full
sudo apt-get install -y ragel

if [[ $BUILD_DOC == "true" ]]; then
	sudo apt-get install -y doxygen
	sudo pip install sphinx sphinx_rtd_theme

	git clone --depth 1 https://github.com/vovkos/doxyrest
fi
