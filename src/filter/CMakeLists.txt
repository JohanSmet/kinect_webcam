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

cmake_minimum_required(VERSION 2.8)

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

set (KINECT_V2_ROOT "${KINECT_SDK_ROOT}/v2.0-PublicPreview1407")
set (KINECT_V2_INC  "${KINECT_V2_ROOT}/inc")

if (${ARCH_64BIT})
	set (KINECT_V1_LIBS "${KINECT_V1_ROOT}/lib/amd64/Kinect10.lib")
	set (KINECT_V2_LIBS "${KINECT_V2_ROOT}/lib/x64/Kinect20.lib")
else()
	set (KINECT_V1_LIBS	"${KINECT_V1_ROOT}/lib/x86/Kinect10.lib")
	set (KINECT_V2_LIBS "${KINECT_V2_ROOT}/lib/x86/Kinect20.lib")
endif()

# kinect webcam
glob_sources(. ../common)

include_directories(${KINECT_V1_INC}
					${KINECT_V2_INC}
					../common)

add_library(kinect_webcam SHARED ${Sources})
target_link_libraries(kinect_webcam strmbase
									${KINECT_V1_LIBS}
									${KINECT_V2_LIBS})
set_property (TARGET kinect_webcam APPEND PROPERTY COMPILE_DEFINITIONS UNICODE _UNICODE)							

add_custom_command(TARGET kinect_webcam POST_BUILD 
				   COMMAND regsvr32 /s $<TARGET_FILE:kinect_webcam>)


install (TARGETS kinect_webcam RUNTIME DESTINATION .)

# vim: set tabstop=4 shiftwidth=4:
