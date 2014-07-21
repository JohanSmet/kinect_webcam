###############################################################################
#
# File  	: 	glob_sources.cmake
#
# Purpose	: 	find all source-files in the specified directories and add 
#				them to the global variable Sources
#
# Copyright (c) 2014	Contributors as noted in the AUTHORS file
#
# This file is licensed under the terms of the MIT license,
# for more details please see LICENSE.txt in the root directory 
# of the provided source or http://opensource.org/licenses/MIT
#
###############################################################################

macro(glob_sources Dirs)
	set(Sources)

	foreach(SourceDir ${ARGV})
		# >> build a list with all the subdirectories
		set(SrcDirs)

		if   (${SourceDir} STREQUAL ".")
			list(APPEND SrcDirs ".")
		else  (${SourceDir} STREQUAL ".")
			file(GLOB_RECURSE ListAllFiles ${SourceDir}/* )

			foreach (FileRec ${ListAllFiles})
				get_filename_component(DirName ${FileRec} PATH)
				file(RELATIVE_PATH DirName ${CMAKE_CURRENT_SOURCE_DIR} ${DirName})
				list(APPEND SrcDirs ${DirName})
			endforeach (FileRec ${ListAllFiles})

			list(REMOVE_DUPLICATES SrcDirs)

		endif (${SourceDir} STREQUAL ".")

		# >> glob the directories and make a nice sourcegroup for each
		foreach(SrcDir ${SrcDirs})
			file (GLOB DirSources 	${SrcDir}/*.cpp 
									${SrcDir}/*.c 
									${SrcDir}/*.m 
									${SrcDir}/*.mm 
									${SrcDir}/*.h 
									${SrcDir}/*.inc
									${SrcDir}/*.def
									${SrcDir}/*.html
									${SrcDir}/*.js)

			list(APPEND Sources ${DirSources})

			if    (${SrcDir} STREQUAL ".")
				set(SourceGroup "sources")
			else  (${SrcDir} STREQUAL ".")
				set(SourceGroup "sources\\${SrcDir}")
				string(REPLACE "/" "\\" SourceGroup ${SourceGroup})
			endif (${SrcDir} STREQUAL ".")

			source_group(${SourceGroup} FILES ${DirSources})
		endforeach(SrcDir ${SrcDirs})

	endforeach(SourceDir)

endmacro(glob_sources)

# vim: set tabstop=4 shiftwidth=4:
