#...............................................................................
#
#  This file is part of the Jancy toolkit.
#
#  Jancy is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/jancy/license.txt
#
#...............................................................................

brew update
brew install coreutils
brew install lua
brew install ragel
brew install p7zip

# coverage should be collected without optimizations

if [ "$BUILD_CONFIGURATION" == "Debug" ]; then
	brew install lcov
	echo "axl_override_setting (GCC_FLAG_COVERAGE -coverage)" >> settings.cmake
fi

if [ "$BUILD_CONFIGURATION" == "Release" ] && [ "$LLVM_VERSION" == "3.4.2" ]; then
	DEPLOY_JANCY_PACKAGE=ON
	BUILD_DOC=ON

	brew install doxygen
	pip install sphinx sphinx_rtd_theme
	rvm get stable
fi

# openssl is already installed, but not linked

echo "set (OPENSSL_INC_DIR /usr/local/opt/openssl/include)" >> paths.cmake
echo "set (OPENSSL_LIB_DIR /usr/local/opt/openssl/lib)" >> paths.cmake
