///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	input/motion/device_factory.cpp
//
// Purpose	: 	create motion input devices
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory 
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#include "device_factory.h"

#ifdef HAVE_KINECT_V2
#include "device_kinect_v2.h"
#endif // HAVE_KINECT_V2

#ifdef HAVE_KINECT_V1
#include "device_kinect.h"
#endif // HAVE_KINECT_V1

#include "device_null.h"

namespace device {

std::unique_ptr<class Device> device_factory(const std::string &p_type)
{
	std::unique_ptr<class Device> f_result = nullptr;

#ifdef HAVE_KINECT_V1
	if (p_type == "kinect")
	{
		f_result = std::make_unique<DeviceKinect>();
		return f_result;
	}
#endif // HAVE_KINECT_V1

#ifdef HAVE_KINECT_V2
	if (p_type == "kinect_v2")
	{
		f_result = std::make_unique<DeviceKinectV2>();
		return f_result;
	}
#endif // HAVE_KINECT_V2

	f_result = std::make_unique<DeviceNull>();
	return std::move(f_result);
}

} // namespace device
