///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	kinect_wrapper.h
//
// Purpose	: 	dynamicly load the kinect library
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory 
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#ifndef KW_KINECT_WRAPPER_H
#define KW_KINECT_WRAPPER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlobj.h>
#include <NuiApi.h>

//
// interface
//

namespace device {

typedef HRESULT (WINAPI *NuiGetSensorCountFunc) (int *pCount);
typedef HRESULT (WINAPI *NuiCreateSensorByIndexFunc) (int index, INuiSensor **ppNuiSensor);
typedef HRESULT (WINAPI *NuiImageGetColorPixelCoordinatesFromDepthPixelFunc) (
						NUI_IMAGE_RESOLUTION eColorResolution,
						CONST NUI_IMAGE_VIEW_AREA *pcViewArea,
						LONG   lDepthX, LONG   lDepthY, USHORT usDepthValue,
						LONG *plColorX, LONG *plColorY);

struct KinectFuncs 
{
	NuiGetSensorCountFunc								NuiGetSensorCount;
	NuiCreateSensorByIndexFunc							NuiCreateSensorByIndex;
	NuiImageGetColorPixelCoordinatesFromDepthPixelFunc	NuiImageGetColorPixelCoordinatesFromDepthPixel;
};

KinectFuncs *kinect_load_library();
void kinect_free_library();

} // namespace device

#endif
