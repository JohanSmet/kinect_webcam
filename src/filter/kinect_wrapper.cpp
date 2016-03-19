///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	kinect_wrapper.cpp
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

#include "kinect_wrapper.h"

namespace {

static device::KinectFuncs	g_kinect_funcs = {0};
static HMODULE				g_kinect_library = nullptr;

} // unnamed namespace

namespace device {

KinectFuncs *kinect_load_library()
{
	if (g_kinect_library != nullptr)
	{
		return &g_kinect_funcs;
	}

	g_kinect_library = LoadLibrary(L"kinect10.dll");

	if (g_kinect_library == nullptr) 
	{
		return nullptr;
	}

	g_kinect_funcs.NuiGetSensorCount = (NuiGetSensorCountFunc) GetProcAddress(g_kinect_library, "NuiGetSensorCount");
	g_kinect_funcs.NuiCreateSensorByIndex = (NuiCreateSensorByIndexFunc) GetProcAddress(g_kinect_library, "NuiCreateSensorByIndex");
	g_kinect_funcs.NuiImageGetColorPixelCoordinatesFromDepthPixel = (NuiImageGetColorPixelCoordinatesFromDepthPixelFunc) GetProcAddress(g_kinect_library, "NuiImageGetColorPixelCoordinatesFromDepthPixel");

	if (g_kinect_funcs.NuiCreateSensorByIndex == nullptr ||
		g_kinect_funcs.NuiGetSensorCount == nullptr ||
		g_kinect_funcs.NuiImageGetColorPixelCoordinatesFromDepthPixel == nullptr)
	{
		kinect_free_library();
		return nullptr;
	}

	return &g_kinect_funcs;
}

void kinect_free_library()
{
	if (g_kinect_library)
	{
		FreeLibrary(g_kinect_library);
		g_kinect_library = nullptr;
	}
}

} // namespace device