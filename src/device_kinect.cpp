///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	device_kinect_cpp
//
// Purpose	: 	interface to the kinect sensor
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory 
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#include "device_kinect.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <vector>
#include <Shlobj.h>
#include <NuiApi.h>

#include "image.h"

namespace device {

struct DeviceKinectPrivate 
{
	INuiSensor *		m_sensor;
	HANDLE				m_sensor_data_event;
	HANDLE				m_sensor_color_stream;

	int					m_color_width;
	int					m_color_height;
	std::vector<BYTE>	m_color_data;

	int					m_focus_joint;
	bool				m_focus_available;
	Point2D				m_focus;
};

//
// construction
//

DeviceKinect::DeviceKinect() :	m_private(std::make_unique<DeviceKinectPrivate>())
{
	m_private->m_sensor				= nullptr;
	m_private->m_sensor_data_event	= INVALID_HANDLE_VALUE;
}

DeviceKinect::~DeviceKinect()
{
}

//
// connection to the device
//

bool DeviceKinect::connect_to_first()
{
	m_private->m_sensor	= nullptr;

	// how many kinect-sensors are connected to the machine ?
	int				f_sensor_count = 0;

	HRESULT f_result = NuiGetSensorCount(&f_sensor_count);
    
	if (FAILED(f_result))
    {
        return false;
    }

	// enumerate all the connected sensors until we find one that works
	for (int f_idx = 0; f_idx < f_sensor_count; ++f_idx)
    {
		// create the device
		f_result = NuiCreateSensorByIndex(f_idx, &m_private->m_sensor);
        if (FAILED(f_result))
        {
            continue;
        }

		// check the status of the device
		f_result = m_private->m_sensor->NuiStatus();

        if (f_result == S_OK)
        {
            break;
        }

		// status-check failed - release the sensor
		m_private->m_sensor->Release();
	}

	// initialize the kinect
	if (m_private->m_sensor != nullptr)
	{
		f_result = m_private->m_sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR); 
        
		if (SUCCEEDED(f_result))
        {
            // create an event that will be signaled when data is available
			m_private->m_sensor_data_event = CreateEventW(NULL, TRUE, FALSE, NULL);
        }

		if (SUCCEEDED(f_result))
        {
			// enable the color stream
			f_result = m_private->m_sensor->NuiImageStreamOpen(
													NUI_IMAGE_TYPE_COLOR,
													NUI_IMAGE_RESOLUTION_640x480,
													0,
													2,
													m_private->m_sensor_data_event,
													&m_private->m_sensor_color_stream);

			m_private->m_color_width  = 640;
			m_private->m_color_height = 480;
			m_private->m_color_data.resize(640 * 480 * 4);
		}

		if (SUCCEEDED(f_result))
		{
			// enable skeletal tracking
			f_result = m_private->m_sensor->NuiSkeletonTrackingEnable(nullptr, NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT);
		}

		if (FAILED(f_result))
		{
			m_private->m_sensor->Release();
			m_private->m_sensor = nullptr;
		}
	}

	if (m_private->m_sensor != nullptr)
	{
		m_private->m_focus_joint	 = NUI_SKELETON_POSITION_HEAD;
		m_private->m_focus_available = false;
		m_private->m_focus			 = {0, 0};
		return true;
	}
    
	return false;
}

bool DeviceKinect::disconnect()
{
	if (m_private->m_sensor_data_event != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_private->m_sensor_data_event);
		m_private->m_sensor_data_event	= INVALID_HANDLE_VALUE;
	}

	if (m_private->m_sensor)
	{
		m_private->m_sensor->NuiShutdown();
		m_private->m_sensor->Release();
		m_private->m_sensor = nullptr;
	}

	m_private->m_color_data.clear();

	return true;
}

//
// video resolutions
//

DeviceVideoResolution DeviceKinect::m_video_resolutions[] = {	{ 320,  240, 32, 30},
																{ 640,  480, 32, 30},
																{ 320,  240, 24, 30},
																{ 640,  480, 24, 30},
															};

int	DeviceKinect::video_resolution_count()
{
	return sizeof(m_video_resolutions) / sizeof(DeviceVideoResolution);
}

int	DeviceKinect::video_resolution_preferred()
{
	return 0;
}

int	DeviceKinect::video_resolution_native()
{
	return 0;
}

DeviceVideoResolution DeviceKinect::video_resolution(int p_index)
{
	return m_video_resolutions[p_index];
} 

//
// body tracking
//


void DeviceKinect::focus_set_joint(int p_joint)
{
	if (p_joint >= 0 && p_joint < NUI_SKELETON_POSITION_COUNT)
		m_private->m_focus_joint = p_joint;
}

bool DeviceKinect::focus_availabe()
{
	return m_private->m_focus_available;
}

Point2D	DeviceKinect::focus_point()
{
	return m_private->m_focus;
}


//
// update detected data
//

bool DeviceKinect::update()
{
	if (!m_private->m_sensor)
		return false;

	// check if there is new data available (don't block)
	if (WaitForSingleObject(m_private->m_sensor_data_event, 0) != WAIT_OBJECT_0)
	{
		return false;									// exit !!!
	}

	// retrieve updated data from the device
	bool f_result = read_color_frame();
	f_result &= read_skeleton_frame();

	return f_result;
}

//
// access to image data
//

bool DeviceKinect::color_data(int p_hor_focus, int p_ver_focus, int p_width, int p_height, int p_bpp, unsigned char *p_data)
{
	if (m_private->m_color_data.empty())
		return false;

	if (p_width  > m_private->m_color_width  ||
	    p_height > m_private->m_color_height)
	{
		return false;
	}

	// offsets
	int f_hor_offset = p_hor_focus - (p_width / 2);
	f_hor_offset	 = min(max(f_hor_offset, 0), m_private->m_color_width - p_width);

	int f_ver_offset = p_ver_focus - (p_height / 2);
	f_ver_offset	 = min(max(f_ver_offset, 0), m_private->m_color_height - p_height);

	switch (p_bpp)
	{	
		case 32 :
			return img::copy_region_32bpp_32bpp(m_private->m_color_width, m_private->m_color_height, m_private->m_color_data.data(),
												f_hor_offset, f_ver_offset, p_width, p_height, p_data);
				
		case 24 :
			return img::copy_region_32bpp_24bpp(m_private->m_color_width, m_private->m_color_height, m_private->m_color_data.data(),
												f_hor_offset, f_ver_offset, p_width, p_height, p_data);
	
		default :
			return false;
	}

}

bool DeviceKinect::read_color_frame()
{
	// attempt to get the color frame
	NUI_IMAGE_FRAME	f_frame;
	HRESULT f_result = m_private->m_sensor->NuiImageStreamGetNextFrame(m_private->m_sensor_color_stream, 0, &f_frame);

    if (FAILED(f_result))
    {
        return false;
    }

	INuiFrameTexture *f_texture = f_frame.pFrameTexture;
    NUI_LOCKED_RECT   f_locked_rect;

    // lock the frame data so the Kinect knows not to modify it while we're reading it
    f_texture->LockRect(0, &f_locked_rect, nullptr, 0);

	// process it
	std::memcpy(m_private->m_color_data.data(), static_cast<BYTE *>(f_locked_rect.pBits), f_locked_rect.size);
			
	// we're done with the texture so unlock it
    f_texture->UnlockRect(0);

    // release the frame
    m_private->m_sensor->NuiImageStreamReleaseFrame(m_private->m_sensor_color_stream, &f_frame);
	return true;
}

bool DeviceKinect::read_skeleton_frame()
{
	// get the kinect skeleton
	NUI_SKELETON_FRAME	f_kinect_skeletons = {0};
	HRESULT				f_result		   = m_private->m_sensor->NuiSkeletonGetNextFrame(0, &f_kinect_skeletons);

    if (FAILED(f_result))
    {
        return false;
    }

	// smooth out the skeleton data (todo: allow the parameters to be tweaked)
	const NUI_TRANSFORM_SMOOTH_PARAMETERS DefaultParams =			{0.5f, 0.5f, 0.5f, 0.05f, 0.04f};
	const NUI_TRANSFORM_SMOOTH_PARAMETERS SomewhatLatentParams =	{0.5f, 0.1f, 0.5f, 0.1f, 0.1f};
	const NUI_TRANSFORM_SMOOTH_PARAMETERS VerySmoothParams =		{0.7f, 0.3f, 1.0f, 1.0f, 1.0f};

    f_result = m_private->m_sensor->NuiTransformSmooth(&f_kinect_skeletons, &SomewhatLatentParams);

	// iterate of the skeletons and focus on the first
	m_private->m_focus_available = false;

	for (auto f_idx = 0; SUCCEEDED(f_result) && !m_private->m_focus_available && f_idx < NUI_SKELETON_COUNT; ++f_idx)
	{
		auto &f_kinect_skeleton = f_kinect_skeletons.SkeletonData[f_idx];

		// is the body tracked ?
		bool f_is_tracked = (f_kinect_skeleton.eTrackingState == NUI_SKELETON_TRACKED);
	
		// convert the location of the focus joint to color space
		if (f_is_tracked)
		{
			LONG	f_depth_x, f_depth_y;
			LONG	f_color_x, f_color_y;
			USHORT	f_depth;

			NuiTransformSkeletonToDepthImage(f_kinect_skeleton.SkeletonPositions[m_private->m_focus_joint], 
											 &f_depth_x, &f_depth_y, &f_depth);

			f_result = NuiImageGetColorPixelCoordinatesFromDepthPixel(	NUI_IMAGE_RESOLUTION_640x480, nullptr, 
																		f_depth_x, f_depth_y, f_depth,
																		&f_color_x, &f_color_y);

			if (SUCCEEDED (f_result))
			{
				m_private->m_focus_available = true;
				m_private->m_focus.m_x		 = f_color_x;
				m_private->m_focus.m_y		 = f_color_y;
			}
		}
	}

	return true;
}

} // namespace device