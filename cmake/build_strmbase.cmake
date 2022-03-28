###############################################################################
#
# File  	: 	CMakeLists.txt
#
# Purpose	:	Build the DirectShow Base Classes
#
# Copyright (c) 2014	Contributors as noted in the AUTHORS file
#
# This file is licensed under the terms of the MIT license,
# for more details please see LICENSE.txt in the root directory
# of the provided source or http://opensource.org/licenses/MIT
#
###############################################################################

cmake_minimum_required(VERSION 3.12)

set (STRMBASE_DIR "strmbase" CACHE PATH "Directory containing the DirectShow Base Classes")

set (STRMBASE_TARGET strmbase)
add_library(${STRMBASE_TARGET} STATIC)

target_sources(${STRMBASE_TARGET} PRIVATE
	${STRMBASE_DIR}/amextra.cpp
	${STRMBASE_DIR}/amextra.h
	${STRMBASE_DIR}/amfilter.cpp
	${STRMBASE_DIR}/amfilter.h
	${STRMBASE_DIR}/amvideo.cpp
	${STRMBASE_DIR}/arithutil.cpp
	${STRMBASE_DIR}/baseclasses.sln
	${STRMBASE_DIR}/baseclasses.vcproj
	${STRMBASE_DIR}/cache.h
	${STRMBASE_DIR}/checkbmi.h
	${STRMBASE_DIR}/combase.cpp
	${STRMBASE_DIR}/combase.h
	${STRMBASE_DIR}/cprop.cpp
	${STRMBASE_DIR}/cprop.h
	${STRMBASE_DIR}/ctlutil.cpp
	${STRMBASE_DIR}/ctlutil.h
	${STRMBASE_DIR}/ddmm.cpp
	${STRMBASE_DIR}/ddmm.h
	${STRMBASE_DIR}/dllentry.cpp
	${STRMBASE_DIR}/dllsetup.cpp
	${STRMBASE_DIR}/dllsetup.h
	${STRMBASE_DIR}/dxmperf.h
	${STRMBASE_DIR}/fourcc.h
	${STRMBASE_DIR}/measure.h
	${STRMBASE_DIR}/msgthrd.h
	${STRMBASE_DIR}/mtype.cpp
	${STRMBASE_DIR}/mtype.h
	${STRMBASE_DIR}/outputq.cpp
	${STRMBASE_DIR}/outputq.h
	${STRMBASE_DIR}/perflog.cpp
	${STRMBASE_DIR}/perflog.h
	${STRMBASE_DIR}/perfstruct.h
	${STRMBASE_DIR}/pstream.cpp
	${STRMBASE_DIR}/pstream.h
	${STRMBASE_DIR}/pullpin.cpp
	${STRMBASE_DIR}/pullpin.h
	${STRMBASE_DIR}/refclock.cpp
	${STRMBASE_DIR}/refclock.h
	${STRMBASE_DIR}/reftime.h
	${STRMBASE_DIR}/renbase.cpp
	${STRMBASE_DIR}/renbase.h
	${STRMBASE_DIR}/schedule.cpp
	${STRMBASE_DIR}/schedule.h
	${STRMBASE_DIR}/seekpt.cpp
	${STRMBASE_DIR}/seekpt.h
	${STRMBASE_DIR}/source.cpp
	${STRMBASE_DIR}/source.h
	${STRMBASE_DIR}/streams.h
	${STRMBASE_DIR}/strmctl.cpp
	${STRMBASE_DIR}/strmctl.h
	${STRMBASE_DIR}/sysclock.cpp
	${STRMBASE_DIR}/sysclock.h
	${STRMBASE_DIR}/transfrm.cpp
	${STRMBASE_DIR}/transfrm.h
	${STRMBASE_DIR}/transip.cpp
	${STRMBASE_DIR}/transip.h
	${STRMBASE_DIR}/videoctl.cpp
	${STRMBASE_DIR}/videoctl.h
	${STRMBASE_DIR}/vtrans.cpp
	${STRMBASE_DIR}/vtrans.h
	${STRMBASE_DIR}/winctrl.cpp
	${STRMBASE_DIR}/winctrl.h
	${STRMBASE_DIR}/winutil.cpp
	${STRMBASE_DIR}/winutil.h
	${STRMBASE_DIR}/wxdebug.cpp
	${STRMBASE_DIR}/wxdebug.h
	${STRMBASE_DIR}/wxlist.cpp
	${STRMBASE_DIR}/wxlist.h
	${STRMBASE_DIR}/wxutil.cpp
	${STRMBASE_DIR}/wxutil.h
)

target_include_directories(${STRMBASE_TARGET} PUBLIC ${STRMBASE_DIR})
target_compile_definitions(${STRMBASE_TARGET} PUBLIC UNICODE _UNICODE)
