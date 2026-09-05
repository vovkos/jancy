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

THIS_DIR=`pwd`

echo "if (NOT \"$CPP_STD\" STREQUAL \"\")" >> settings.cmake
echo "    axl_override_setting (GCC_FLAG_CPP_STANDARD -std=$CPP_STD)" >> settings.cmake
echo "endif ()" >> settings.cmake

mkdir axl/build
pushd axl/build
cmake .. -DTARGET_CPU=$TARGET_CPU -DCMAKE_BUILD_TYPE=$BUILD_CONFIGURATION
make
popd

echo "set (AXL_CMAKE_DIR $THIS_DIR/axl/cmake $THIS_DIR/axl/build/cmake)" >> paths.cmake

mkdir graco/build
pushd graco/build
cmake .. -DTARGET_CPU=$TARGET_CPU -DCMAKE_BUILD_TYPE=$BUILD_CONFIGURATION
make
popd

echo "set (GRACO_CMAKE_DIR $THIS_DIR/graco/cmake $THIS_DIR/graco/build/cmake)" >> paths.cmake

if [ "$BUILD_DOC" != "" ]; then
	mkdir doxyrest/build
	pushd doxyrest/build
	cmake .. -DTARGET_CPU=$TARGET_CPU -DCMAKE_BUILD_TYPE=$BUILD_CONFIGURATION
	make
	popd

	echo "set (DOXYREST_CMAKE_DIR $THIS_DIR/doxyrest/cmake $THIS_DIR/doxyrest/build/cmake)" >> paths.cmake
fi

mkdir build
pushd build
cmake .. -DTARGET_CPU=$TARGET_CPU -DCMAKE_BUILD_TYPE=$BUILD_CONFIGURATION
make
ctest --output-on-failure

if [ "$DEPLOY_JANCY_PACKAGE" == "ON" ]; then
	cpack -G TXZ -D CPACK_PACKAGE_FILE_NAME=jancy-${TRAVIS_OS_NAME}-${TARGET_CPU}
fi

popd

if [ "$GET_COVERAGE" != "" ]; then
	lcov --capture --directory . --no-external --output-file coverage.info
	lcov --remove coverage.info '*/llvm/*' '*/axl/*' '*/graco/*' --output-file coverage.info
	lcov --list coverage.info

	curl -s https://codecov.io/bash | bash
fi

if [ "$BUILD_DOC" != "" ]; then
	pushd build/doc

	source index/build-html.sh
	source build-guide/build-html.sh
	source language/build-html.sh
	source compiler/build-html.sh
	source grammar/build-llk.sh
	source grammar/build-html.sh
	source stdlib/build-xml.sh
	source stdlib/build-rst.sh
	source stdlib/build-html.sh
	source api/build-xml.sh
	source api/build-rst.sh
	source api/build-html.sh

	touch html/.nojekyll
	popd
fi
