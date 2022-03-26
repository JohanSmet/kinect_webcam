///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	kinect_v2_wrapper.cpp
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

#include "kinect_v2_wrapper.h"

namespace {

static device::Kinect2Funcs	g_kinect_funcs = {0};
static HMODULE				g_kinect_library = nullptr;

} // unnamed namespace

namespace device {

Kinect2Funcs *kinect_v2_load_library()
{
	if (g_kinect_library != nullptr)
	{
		return &g_kinect_funcs;
	}

	g_kinect_library = LoadLibrary(L"kinect20.dll");

	if (g_kinect_library == nullptr)
	{
		return nullptr;
	}

	g_kinect_funcs.GetDefaultKinectSensor = (GetDefaultKinectSensorFunc) GetProcAddress(g_kinect_library, "GetDefaultKinectSensor");

	if (g_kinect_funcs.GetDefaultKinectSensor == nullptr)
	{
		kinect_v2_free_library();
		return nullptr;
	}

	return &g_kinect_funcs;
}

void kinect_v2_free_library()
{
	if (g_kinect_library)
	{
		FreeLibrary(g_kinect_library);
		g_kinect_library = nullptr;
	}
}

} // namespace device
