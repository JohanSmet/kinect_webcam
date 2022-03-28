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

cmake_minimum_required(VERSION 3.12)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# external packages
find_package(Qt5 COMPONENTS Widgets Gui)

# QT configuration
set(CMAKE_AUTOMOC ON)
QT5_WRAP_UI(UI_SOURCES mainwindow.ui)

# kw_config
set (UI_TARGET kw_config)
add_executable(${UI_TARGET} WIN32)

target_sources(${UI_TARGET} PRIVATE
	../common/com_utils.h
	../common/guid_filter.cpp
	../common/guid_filter.h
	../common/settings.cpp
	../common/settings.h
	../common/settings_list.h

	com_register.cpp
	com_register.h
	ds_capture.cpp
	ds_capture.h
	main.cpp
	mainwindow.cpp
	mainwindow.h
	mainwindow.ui
	qt_utils.cpp
	qt_utils.h
)

target_include_directories(${UI_TARGET} PRIVATE ../common)
target_link_libraries(${UI_TARGET} strmiids.lib)
target_link_libraries(${UI_TARGET} Qt5::Widgets Qt5::Gui Qt5::GuiPrivate)

set_target_properties(${UI_TARGET} PROPERTIES LINK_FLAGS "/ENTRY:\"mainCRTStartup\"")
set_target_properties(${UI_TARGET} PROPERTIES WIN32_EXECUTABLE TRUE)

target_compile_definitions(${UI_TARGET} PUBLIC UNICODE _UNICODE)

# installation
install (TARGETS ${UI_TARGET} RUNTIME DESTINATION .)

# vim: set tabstop=4 shiftwidth=4:
