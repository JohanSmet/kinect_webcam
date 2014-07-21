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

cmake_minimum_required(VERSION 2.6)

include_directories(strmbase)

glob_sources(strmbase)
add_library(strmbase STATIC ${Sources})
