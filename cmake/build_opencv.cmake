set (BUILD_PERF_TESTS OFF)
set (BUILD_TESTS OFF)
set (BUILD_opencv_apps OFF)
set (BUILD_SHARED_LIBS OFF)
set (BUILD_WITH_STATIC_CRT OFF)

add_subdirectory(opencv)

include_directories (	opencv/include	
			opencv/modules/core/include 
			opencv/modules/imgproc/include
			opencv/modules/photo/include
			opencv/modules/video/include
			opencv/modules/features2d/include
			opencv/modules/objdetect/include
			opencv/modules/calib3d/include
			opencv/modules/imgcodecs/include
			opencv/modules/videoio/include
			opencv/modules/highgui/include
			opencv/modules/ml/include
			opencv/modules/flann/include
		    )

