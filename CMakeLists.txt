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

cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

include (cmake/glob_sources.cmake)

project(kinect_webcam)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output/library)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output/library)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/output/runtime)
set(CMAKE_DEBUG_POSTFIX "")
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".ax")

include (cmake/build_strmbase.cmake)
add_subdirectory(src)

# vim: set tabstop=4 shiftwidth=4:
