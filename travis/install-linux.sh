#!/bin/bash

# manually install CMake -- we need at least CMake 3.3

CMAKE_VERSION=3.3.2
CMAKE_VERSION_DIR=v3.3
CMAKE_OS=Linux-x86_64
CMAKE_TAR=cmake-$CMAKE_VERSION-$CMAKE_OS.tar.gz
CMAKE_URL=http://www.cmake.org/files/$CMAKE_VERSION_DIR/$CMAKE_TAR
CMAKE_DIR=$(pwd)/cmake-$CMAKE_VERSION

wget --quiet $CMAKE_URL
mkdir -p $CMAKE_DIR
tar --strip-components=1 -xzf $CMAKE_TAR -C $CMAKE_DIR
export PATH=$CMAKE_DIR/bin:$PATH

# now to official APT packages

if [ "$TARGET_CPU" != "x86" ]; then
	sudo apt-get -qq update

	sudo apt-get install -y liblua5.2-dev
	sudo apt-get install -y libpcap-dev
	sudo apt-get install -y libudev-dev
else
	sudo dpkg --add-architecture i386
	sudo apt-get -qq update

	sudo apt-get install -y liblua5.2-dev:i386
	sudo apt-get install -y libpcap0.8-dev:i386
	sudo apt-get install -y libudev-dev:i386

	# OpenSSL is already installed, but 64-bit only

	sudo apt-get install -y libssl-dev:i386

	# install g++-multilib -- in the end, after i386 packages!

	sudo apt-get install -y g++-multilib

	# CMake fails to properly switch between 32-bit and 64-bit libraries on Ubuntu

	echo "set (OPENSSL_LIB_DIR /usr/lib/i386-linux-gnu)" >> paths.cmake
	echo "set (EXPAT_LIB_DIR /usr/lib/i386-linux-gnu)" >> paths.cmake
	echo "set (LUA_LIB_DIR /usr/lib/i386-linux-gnu)" >> paths.cmake
	echo "set (PCAP_LIB_DIR /usr/lib/i386-linux-gnu)" >> paths.cmake
	echo "set (EXPAT_INC_DIR DISABLED)" >> paths.cmake

	export LLVM_BUILD_32_BITS="-DLLVM_BUILD_32_BITS=TRUE"
fi

sudo apt-get install -y p7zip-full
