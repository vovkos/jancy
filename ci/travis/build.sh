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
cd axl/build
cmake .. -DTARGET_CPU=$TARGET_CPU -DCMAKE_BUILD_TYPE=$BUILD_CONFIGURATION
make

cd $THIS_DIR
echo "set (AXL_CMAKE_DIR $THIS_DIR/axl/cmake $THIS_DIR/axl/build/cmake)" >> paths.cmake

mkdir graco/build
cd graco/build
cmake .. -DTARGET_CPU=$TARGET_CPU -DCMAKE_BUILD_TYPE=$BUILD_CONFIGURATION
make

cd $THIS_DIR
echo "set (GRACO_CMAKE_DIR $THIS_DIR/graco/cmake $THIS_DIR//graco/build/cmake)" >> paths.cmake

if [ "$BUILD_DOC" != "" ]; then
	mkdir doxyrest/build
	cd doxyrest/build
	cmake .. -DTARGET_CPU=$TARGET_CPU -DCMAKE_BUILD_TYPE=$BUILD_CONFIGURATION
	make

	cd $THIS_DIR
	echo "set (DOXYREST_CMAKE_DIR $THIS_DIR/doxyrest/cmake $THIS_DIR//doxyrest/build/cmake)" >> paths.cmake
fi

echo "set (LLVM_CMAKE_DIR $THIS_DIR/llvm/$LLVM_CMAKE_SUBDIR)" >> paths.cmake

mkdir build
cd build
cmake .. -DTARGET_CPU=$TARGET_CPU -DCMAKE_BUILD_TYPE=$BUILD_CONFIGURATION
make

if [ "$BUILD_DOC" == "" ]; then
	ctest --output-on-failure
	doc/stdlib/build-xml.sh

	cd $THIS_DIR
	source ci/travis/get-coverage.sh
else
	source doc/index/build-html.sh
	source doc/build-guide/build-html.sh
	source doc/language/build-html.sh
	source doc/compiler/build-html.sh
	source doc/grammar/build-html.sh
	source doc/stdlib/build-xml.sh
	source doc/stdlib/build-rst.sh
	source doc/stdlib/build-html.sh
	source doc/api/build-xml.sh
	source doc/api/build-rst.sh
	source doc/api/build-html.sh

	touch doc/html/.nojekyll
fi
