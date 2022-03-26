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

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 COMPONENTS Widgets Gui)

set(CMAKE_AUTOMOC ON)

glob_sources(. ../common)

QT5_WRAP_UI(UI_SOURCES
	mainwindow.ui)

include_directories(../common)

add_executable(kw_config WIN32 ${Sources} ${UI_SOURCES})
target_link_libraries(kw_config strmiids.lib)
target_link_libraries(kw_config Qt5::Widgets Qt5::Gui Qt5::GuiPrivate)

set_property (TARGET kw_config PROPERTY LINK_FLAGS "/ENTRY:\"mainCRTStartup\"")
set_target_properties(kw_config PROPERTIES WIN32_EXECUTABLE TRUE)

set_property (TARGET kw_config APPEND PROPERTY COMPILE_DEFINITIONS UNICODE _UNICODE)

set(target_location C:/projects/GitHub/kinect_webcam/build/output/runtime/Debug/kw_config.exe)

# installation
set (INSTALL_APP "\${CMAKE_INSTALL_PREFIX}/kw_config.exe")
install (TARGETS kw_config RUNTIME DESTINATION .)

# vim: set tabstop=4 shiftwidth=4:
