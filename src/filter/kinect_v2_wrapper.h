///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	kinect_v2_wrapper.h
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

#ifndef KW_KINECT_V2_WRAPPER_H
#define KW_KINECT_V2_WRAPPER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Kinect.h>

//
// interface
//

namespace device {

typedef HRESULT (WINAPI *GetDefaultKinectSensorFunc) (_COM_Outptr_ IKinectSensor** defaultKinectSensor);

struct Kinect2Funcs
{
	GetDefaultKinectSensorFunc	GetDefaultKinectSensor;
};

Kinect2Funcs *kinect_v2_load_library();
void kinect_v2_free_library();

} // namespace device

#endif
