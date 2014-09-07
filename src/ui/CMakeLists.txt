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

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets)
find_package(Qt5Gui)

set(CMAKE_AUTOMOC ON)

# kinect config
glob_sources(. ../common)

QT5_WRAP_UI(UI_SOURCES
	mainwindow.ui)

include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS}
					../common)

add_executable(kw_config WIN32 ${Sources} ${UI_SOURCES})
qt5_use_modules(kw_config Widgets)
target_link_libraries(kw_config strmiids.lib)

set_property (TARGET kw_config PROPERTY LINK_FLAGS "/ENTRY:\"mainCRTStartup\"")
set_target_properties(kw_config PROPERTIES WIN32_EXECUTABLE TRUE)

set_property (TARGET kw_config APPEND PROPERTY COMPILE_DEFINITIONS UNICODE _UNICODE)							

# vim: set tabstop=4 shiftwidth=4:
