###############################################################################
#
# File  	: 	CMakeLists.txt
#
# Copyright (c) 2014	Contributors as noted in the AUTHORS file
#
# This file is licensed under the terms of the MIT license,
# for more details please see LICENSE.txt in the root directory
# of the provided source or http://opensource.org/licenses/MIT
#
###############################################################################

cmake_minimum_required(VERSION 3.12)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

project(kinect_webcam)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/install)
set(CMAKE_DEBUG_POSTFIX "")
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".ax")

include (cmake/build_strmbase.cmake)

add_subdirectory(src/filter)
add_subdirectory(src/ui)

# vim: set tabstop=4 shiftwidth=4:
