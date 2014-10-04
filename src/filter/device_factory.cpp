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

#include "device_kinect_v2.h"
#include "device_kinect.h"
#include "device_null.h"

namespace device {

std::unique_ptr<class Device> device_factory(const std::string &p_type)
{
	std::unique_ptr<class Device> f_result = nullptr;

	if (p_type == "kinect")
	{
		f_result = std::make_unique<DeviceKinect>();
	}
	else if (p_type == "kinect_v2")
	{
		f_result = std::make_unique<DeviceKinectV2>();
	}
	else
	{
		f_result = std::make_unique<DeviceNull>();
	}

	return std::move(f_result);
}

} // namespace device
