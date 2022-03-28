###############################################################################
#
# File  	: 	CmakeLists.txt
#
# Copyright (c) 2014	Contributors as noted in the AUTHORS file
#
# This file is licensed under the terms of the MIT license,
# for more details please see LICENSE.txt in the root directory
# of the provided source or http://opensource.org/licenses/MIT
#
###############################################################################

cmake_minimum_required(VERSION 2.8.12)

# configuration
option(ENABLE_KINECT_V1 "Enable the Kinect V1 SDK" OFF)
option(ENABLE_KINECT_V2 "Enable the Kinect V2 SDK" OFF)

if(DEFINED ENV{ProgramW6432})
	set (KINECT_SDK_ROOTDIR "$ENV{ProgramW6432}/Microsoft SDKs/Kinect" CACHE PATH
			"Directory containing the Kinect SDK")
else()
	set (KINECT_SDK_ROOTDIR "$ENV{ProgramFiles}/Microsoft SDKs/Kinect" CACHE PATH
			"Directory containing the Kinect SDK")
endif()

set (KINECT_V1_VERSION "v1.8" CACHE STRING "Version of the Kinect v1 SDK to use")
set (KINECT_V2_VERSION "v2.0_1409" CACHE STRING "Version of the Kinect v2 SDK to use")

# check for a 64bit platform
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH_64BIT	true)
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(ARCH_64BIT	false)
else()
	message (FATAL_ERROR "Pointers aren't 64bit or 32bit ...")
endif()

# external packages
find_package(OpenCV)

# kinect webcam
set (FILTER_TARGET kinect_webcam)
add_library(${FILTER_TARGET} SHARED ${Sources})

target_sources(${FILTER_TARGET} PRIVATE
	../common/com_utils.h
	../common/guid_filter.cpp
	../common/guid_filter.h
	../common/settings.cpp
	../common/settings.h
	../common/settings_list.h

	device.h
	device_factory.cpp
	device_factory.h
	device_null.cpp
	device_null.h
	dll.cpp
	filter_video.cpp
	filter_video.h
	filters.def
	image.cpp
	image.h
)

if (ENABLE_KINECT_V1)
	target_sources(${FILTER_TARGET} PRIVATE
		device_kinect.cpp
		device_kinect.h
		kinect_wrapper.cpp
		kinect_wrapper.h
	)
	target_compile_definitions(${FILTER_TARGET} PRIVATE HAVE_KINECT_V1)
	target_include_directories(${FILTER_TARGET} PRIVATE ${KINECT_SDK_ROOTDIR}/${KINECT_V1_VERSION}/inc)
endif()

if (ENABLE_KINECT_V2)
	target_sources(${FILTER_TARGET} PRIVATE
		device_kinect_v2.cpp
		device_kinect_v2.h
		kinect_v2_wrapper.cpp
		kinect_v2_wrapper.h
	)
	target_compile_definitions(${FILTER_TARGET} PRIVATE HAVE_KINECT_V2)
	target_include_directories(${FILTER_TARGET} PRIVATE ${KINECT_SDK_ROOTDIR}/${KINECT_V2_VERSION}/inc)
endif()

target_link_libraries(${FILTER_TARGET} PRIVATE ${STRMBASE_TARGET})
target_link_libraries(${FILTER_TARGET} PRIVATE opencv::core opencv::imgproc)
target_include_directories(${FILTER_TARGET} PRIVATE ../common)

# installation
install (TARGETS ${FILTER_TARGET} RUNTIME DESTINATION .)

# vim: set tabstop=4 shiftwidth=4:
