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

namespace device {

struct DeviceKinectPrivate 
{
	INuiSensor *		m_sensor;
	HANDLE				m_sensor_data_event;
	HANDLE				m_sensor_color_stream;

	int					m_color_width;
	int					m_color_height;
	std::vector<BYTE>	m_color_data;
};

//
// construction
//

DeviceKinect::DeviceKinect() :	m_private(std::make_unique<DeviceKinectPrivate>())
{
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
		f_result = m_private->m_sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_COLOR); 
        
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

		if (FAILED(f_result))
		{
			m_private->m_sensor->Release();
			m_private->m_sensor = nullptr;
		}
	}

	if (m_private->m_sensor != nullptr)
	{
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
	}

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

DeviceVideoResolution DeviceKinect::video_resolution(int p_index)
{
	return m_video_resolutions[p_index];
} 

//
// update detected data
//

bool DeviceKinect::update()
{
	// check if there is new data available (don't block)
	if (WaitForSingleObject(m_private->m_sensor_data_event, 0)	!= WAIT_OBJECT_0)
	{
		return false;									// exit !!!
	}

	// retrieve updated data from the device
	return read_color_frame();
}

//
// access to image data
//

bool DeviceKinect::color_data(int p_width, int p_height, int p_bpp, unsigned char *p_data)
{
	if (p_width  > m_private->m_color_width  ||
	    p_height > m_private->m_color_height)
	{
		return false;
	}

	// offsets
	int f_hor_offset = (m_private->m_color_width - p_width) / 2;
	int f_ver_offset = (m_private->m_color_height - p_height) / 2;

	switch (p_bpp)
	{	
		case 32 :
			return color_data_32bpp(f_hor_offset, f_ver_offset, p_width, p_height, p_data);

		case 24 :
			return color_data_24bpp(f_hor_offset, f_ver_offset, p_width, p_height, p_data);
	
		default :
			return false;
	}
}

bool DeviceKinect::color_data_32bpp(int p_hor_offset, int p_ver_offset, int p_width, int p_height, unsigned char *p_data)
{
	const int	f_pixel_size	= 4;
	int			f_line_size		= p_width * f_pixel_size;
	int			f_line_stride	= m_private->m_color_width * f_pixel_size;
	const auto *f_src_start		= m_private->m_color_data.data() + (p_hor_offset * f_pixel_size) + (p_ver_offset * f_line_stride);

	// swap the picture vertically
	for (const auto *f_src_line = f_src_start + (f_line_stride * (p_height - 1));	// start of the last line
		 f_src_line >= f_src_start;
		 f_src_line -= f_line_stride, p_data += f_line_size)
	{
		memcpy(p_data, f_src_line, f_line_size);
	}

	return true;
}

bool DeviceKinect::color_data_24bpp(int p_hor_offset, int p_ver_offset, int p_width, int p_height, unsigned char *p_data)
{
	const int	f_s_pixel_size	= 4;
	const int	f_d_pixel_size	= 3;
	const int	f_d_line_size	= p_width * f_d_pixel_size;
	const int	f_s_line_stride	= m_private->m_color_width * f_s_pixel_size;

	const auto *f_src_start		= m_private->m_color_data.data() + (p_hor_offset * f_s_pixel_size) + (p_ver_offset * f_s_line_stride);

	// swap the picture vertically
	for (const auto *f_src_line = f_src_start + (f_s_line_stride * (p_height - 1));	// start of the last line
		 f_src_line >= f_src_start;
		 f_src_line -= f_s_line_stride, p_data += f_d_line_size)
	{
		auto *f_src = f_src_line;
		auto *f_dst = p_data;

		for (int f_w = 0; f_w < p_width; ++f_w)
		{
			*f_dst++ = *f_src++;
			*f_dst++ = *f_src++;
			*f_dst++ = *f_src++;
			++f_src;
		}
	}

	return true;
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

} // namespace device