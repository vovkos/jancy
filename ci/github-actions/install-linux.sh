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

sudo apt-get remove -y --purge '^llvm(-[0-9]+)?-dev(:i386)?$'

if [[ $TARGET_CPU != "x86" ]]; then
	sudo apt-get -qq update

	sudo apt-get install -y liblua5.2-dev
	sudo apt-get install -y libpcap-dev
	sudo apt-get install -y libudev-dev
	sudo apt-get install -y libusb-1.0
	sudo apt-get install -y ${LLVM_VERSION}-dev
	sudo apt-get install -y libz-dev
else
	sudo dpkg --add-architecture i386
	sudo apt-get -qq update

	sudo apt-get remove -y python3

	echo installing...

	sudo apt-get install -y liblua5.2-dev:i386
	sudo apt-get install -y libpcap-dev:i386
	sudo apt-get install -y libudev-dev:i386
	sudo apt-get install -y libusb-1.0:i386
	sudo apt-get install -y python3:i386
	sudo apt-get install -y ${LLVM_VERSION}-dev:i386
	sudo apt-get install -y zlib1g-dev:i386
	sudo apt-get install -y libssl-dev:i386

	# install g++-multilib -- in the end, after i386 packages!

	sudo apt-get install -y g++-multilib

	# CMake fails to properly switch between 32-bit and 64-bit libraries on Ubuntu

	echo "set (OPENSSL_INC_DIR /usr/include/i386-linux-gnu)" >> paths.cmake
	echo "set (OPENSSL_LIB_DIR /usr/lib/i386-linux-gnu)" >> paths.cmake
	echo "set (LUA_LIB_DIR /usr/lib/i386-linux-gnu)" >> paths.cmake
	echo "set (PCAP_LIB_DIR /usr/lib/i386-linux-gnu)" >> paths.cmake
	echo "set (LIBUSB_LIB_DIR /usr/lib/i386-linux-gnu)" >> paths.cmake
	echo "set (ZLIB_ROOT /usr/lib/i386-linux-gnu)" >> paths.cmake
	echo "set (EXPAT_INC_DIR DISABLED)" >> paths.cmake
fi

sudo apt-get install -y p7zip-full
sudo apt-get install -y ragel

if [[ $BUILD_DOC == "true" ]]; then
	sudo apt-get install -y doxygen
	sudo pip install sphinx sphinx_rtd_theme

	git clone --depth 1 https://github.com/vovkos/doxyrest
fi
