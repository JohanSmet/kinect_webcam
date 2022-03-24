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

option(ENABLE_KINECT_V1 "Enable the Kinect V1 SDK" OFF)
option(ENABLE_KINECT_V2 "Enable the Kinect V2 SDK" OFF)

# check for a 64bit platform
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(ARCH_64BIT	true)
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
	set(ARCH_64BIT	false)
else()
	message (FATAL_ERROR "Pointers aren't 64bit or 32bit ...")
endif()

# kinect SDKs
if(DEFINED ENV{ProgramW6432})
	set (KINECT_SDK_ROOT "$ENV{ProgramW6432}/Microsoft SDKs/Kinect")
else()
	set (KINECT_SDK_ROOT "$ENV{ProgramFiles}/Microsoft SDKs/Kinect")
endif()

set (KINECT_V1_ROOT "${KINECT_SDK_ROOT}/v1.8")
set (KINECT_V1_INC  "${KINECT_V1_ROOT}/inc")

set (KINECT_V2_ROOT "${KINECT_SDK_ROOT}/v2.0_1409")
set (KINECT_V2_INC  "${KINECT_V2_ROOT}/inc")

# kinect webcam
include_directories(${KINECT_V1_INC}
					${KINECT_V2_INC} 
					${OpenCV_INCLUDE_DIRS}
					../common)

add_library(kinect_webcam SHARED ${Sources})
target_sources(kinect_webcam PRIVATE
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
	target_sources(kinect_webcam PRIVATE
		device_kinect.cpp
		device_kinect.h
		kinect_wrapper.cpp
		kinect_wrapper.h
	)
	target_compile_definitions(kinect_webcam PRIVATE HAVE_KINECT_V1)
endif()

if (ENABLE_KINECT_V2)
	target_sources(kinect_webcam PRIVATE
		device_kinect_v2.cpp
		device_kinect_v2.h
		kinect_v2_wrapper.cpp
		kinect_v2_wrapper.h
	)
	target_compile_definitions(kinect_webcam PRIVATE HAVE_KINECT_V2)
endif()

target_link_libraries(kinect_webcam strmbase
									opencv_core opencv_imgproc)
set_property (TARGET kinect_webcam APPEND PROPERTY COMPILE_DEFINITIONS UNICODE _UNICODE)							

install (TARGETS kinect_webcam RUNTIME DESTINATION .)

# vim: set tabstop=4 shiftwidth=4:
