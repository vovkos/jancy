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

brew update
brew install coreutils
brew install lua
brew install ragel
brew install p7zip
brew install llvm@8

# openssl is already installed, but not linked

echo "set (OPENSSL_INC_DIR /usr/local/opt/openssl/include)" >> paths.cmake
echo "set (OPENSSL_LIB_DIR /usr/local/opt/openssl/lib)" >> paths.cmake

# homebrew llvm installation is not linked

echo "set (LLVM_CMAKE_DIR /usr/local/opt/llvm@8/lib/cmake/llvm)" >> paths.cmake

# llvm-8 requires c++11

echo "axl_override_setting (GCC_FLAG_CPP_STANDARD -std=c++11)" >> settings.cmake
