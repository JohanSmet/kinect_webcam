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

find_package(Qt5Widgets)
find_package(Qt5Gui)

set(CMAKE_AUTOMOC ON)

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

set(target_location C:/projects/GitHub/kinect_webcam/build/output/runtime/Debug/kw_config.exe)

###############################################################################
# 
# installation
#

# -> application
set (INSTALL_APP "\${CMAKE_INSTALL_PREFIX}/kw_config.exe")
install (TARGETS kw_config RUNTIME DESTINATION .)

# -> QT5 (get the installation path of the QT5 library)
get_target_property(QT5_LIB_LOCATION Qt5::Widgets LOCATION_${CMAKE_BUILD_TYPE})
get_filename_component(QT5_LIB_PATH ${QT5_LIB_LOCATION} DIRECTORY)
get_filename_component(QT5_PLUGIN_PATH "${QT5_LIB_PATH}/../plugins" ABSOLUTE)

install(FILES "${QT5_PLUGIN_PATH}/platforms/qwindows.dll" DESTINATION platforms COMPONENT Runtime CONFIGURATIONS Release;RelWithDebInfo;MinSizeRel)
install(FILES "${QT5_PLUGIN_PATH}/platforms/qwindowsd.dll" DESTINATION platforms COMPONENT Runtime CONFIGURATIONS Debug)

install(CODE "
	file(GLOB_RECURSE QT5_PLUGINS
		 \"\${CMAKE_INSTALL_PREFIX}/platforms/*.dll\")
    include(BundleUtilities)
    fixup_bundle(\"${INSTALL_APP}\" \"\${QT5_PLUGINS}\" \"${QT5_LIB_PATH}\")
    " COMPONENT Runtime)

# vim: set tabstop=4 shiftwidth=4:
