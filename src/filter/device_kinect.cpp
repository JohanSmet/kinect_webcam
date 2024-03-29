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

#include <vector>

#include "kinect_wrapper.h"
#include "image.h"
#include "com_utils.h"

namespace device {

struct DeviceKinectPrivate
{
	KinectFuncs	*			m_kinect_lib;
	INuiSensor *			m_sensor;
	HANDLE					m_sensor_data_event;
	HANDLE					m_sensor_color_stream;
	HANDLE					m_sensor_depth_stream;
	INuiCoordinateMapper *	m_sensor_coordinate_mapper;

	int						m_color_width;
	int						m_color_height;
	std::vector<BYTE>		m_color_data;
	DevicePixelFormat		m_color_format;
	NUI_IMAGE_TYPE			m_nui_color_type;
    NUI_IMAGE_RESOLUTION	m_nui_color_resolution;

	bool					m_flip_output;
	bool					m_high_res;
	bool					m_green_screen;

	int									m_depth_width;
	int									m_depth_height;
	std::vector<NUI_DEPTH_IMAGE_PIXEL>	m_depth_data;
    NUI_IMAGE_RESOLUTION				m_nui_depth_resolution;

	std::vector<BYTE>					m_body_mask;
	std::vector<NUI_DEPTH_IMAGE_POINT>	m_depth_points;

	int									m_focus_joint;
	bool								m_focus_available;
	Point2D								m_focus;
};

//
// construction
//

DeviceKinect::DeviceKinect() :	m_private(std::make_unique<DeviceKinectPrivate>())
{
	m_private->m_sensor				= nullptr;
	m_private->m_sensor_data_event	= INVALID_HANDLE_VALUE;
	m_private->m_flip_output		= false;
	m_private->m_green_screen		= false;
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

	// try to load the kinect library
	m_private->m_kinect_lib = kinect_load_library();

	if (m_private->m_kinect_lib == nullptr)
	{
		return false;
	}

	// how many kinect-sensors are connected to the machine ?
	int				f_sensor_count = 0;

	HRESULT f_result = m_private->m_kinect_lib->NuiGetSensorCount(&f_sensor_count);

	if (FAILED(f_result))
    {
        return false;
    }

	// enumerate all the connected sensors until we find one that works
	for (int f_idx = 0; f_idx < f_sensor_count; ++f_idx)
    {
		// create the device
		f_result = m_private->m_kinect_lib->NuiCreateSensorByIndex(f_idx, &m_private->m_sensor);
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
		f_result = m_private->m_sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX);

		if (SUCCEEDED(f_result))
        {
            // create an event that will be signaled when data is available
			m_private->m_sensor_data_event = CreateEventW(NULL, TRUE, FALSE, NULL);
        }

		if (SUCCEEDED(f_result))
        {
			// enable the color stream
			init_color_stream(m_private->m_color_format, m_private->m_high_res);
		}

		if (SUCCEEDED(f_result))
        {
			// enable the depth stream
			init_depth_stream();
		}

		if (SUCCEEDED(f_result))
		{
			// enable skeletal tracking
			f_result = m_private->m_sensor->NuiSkeletonTrackingEnable(nullptr, NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT);
		}

		if (SUCCEEDED(f_result))
		{
			f_result = m_private->m_sensor->NuiGetCoordinateMapper(&m_private->m_sensor_coordinate_mapper);
		}

		if (FAILED(f_result))
		{
			com_safe_release(&m_private->m_sensor_coordinate_mapper);
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

	com_safe_release(&m_private->m_sensor_coordinate_mapper);

	if (m_private->m_sensor)
	{
		m_private->m_sensor->NuiShutdown();
		m_private->m_sensor->Release();
		m_private->m_sensor = nullptr;
	}

	m_private->m_color_data.clear();

	kinect_free_library();

	return true;
}

//
// video resolutions
//

DeviceVideoResolution DeviceKinect::m_video_resolutions[] = {	{ 320,  240, 32, 30, DPF_RGBA},
																{ 640,  480, 32, 30, DPF_RGBA},
																{1280,  960, 32, 12, DPF_RGBA},
																{ 320,  240, 24, 30, DPF_RGB},
																{ 640,  480, 24, 30, DPF_RGB},
																{ 640,  480, 16, 15, DPF_YUY2}
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

void DeviceKinect::video_set_resolution(DeviceVideoResolution p_devres)
{
	m_private->m_high_res	  = (p_devres.m_width > 640);
	m_private->m_color_format = p_devres.m_pixel_format;
}

void DeviceKinect::video_flip_output(bool p_flip)
{
	m_private->m_flip_output = p_flip;
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
// green screen
//

void DeviceKinect::green_screen_enable(bool p_enable)
{
	m_private->m_green_screen = p_enable;
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
	f_result &= read_depth_frame();
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

	if (m_private->m_green_screen)
		build_index_mask();

	switch (p_bpp)
	{
		case 32 :
			if (m_private->m_green_screen)
				return img::copy_region_32bpp_32bpp_mask(	m_private->m_color_width, m_private->m_color_height, m_private->m_color_data.data(), m_private->m_body_mask.data(),
															f_hor_offset, f_ver_offset, p_width, p_height, p_data,
															m_private->m_flip_output);
			else
				return img::copy_region_32bpp_32bpp(m_private->m_color_width, m_private->m_color_height, m_private->m_color_data.data(),
													f_hor_offset, f_ver_offset, p_width, p_height, p_data,
													m_private->m_flip_output);

		case 24 :
			if (m_private->m_green_screen)
				return img::copy_region_32bpp_24bpp_mask(	m_private->m_color_width, m_private->m_color_height, m_private->m_color_data.data(), m_private->m_body_mask.data(),
															f_hor_offset, f_ver_offset, p_width, p_height, p_data,
															m_private->m_flip_output);
			else
				return img::copy_region_32bpp_24bpp(m_private->m_color_width, m_private->m_color_height, m_private->m_color_data.data(),
													f_hor_offset, f_ver_offset, p_width, p_height, p_data,
													m_private->m_flip_output);

		default :
			return false;
	}

}

bool DeviceKinect::init_color_stream(DevicePixelFormat p_format, bool p_high_res)
{
	HRESULT					f_result = S_OK;
	NUI_IMAGE_TYPE			f_img_type = NUI_IMAGE_TYPE_COLOR;
	NUI_IMAGE_RESOLUTION	f_img_res  = NUI_IMAGE_RESOLUTION_640x480;

	m_private->m_color_width  = 640;
	m_private->m_color_height = 480;
	int f_bytes				  = 4;

	if (p_format == DPF_YUY2)
	{
		f_img_type = NUI_IMAGE_TYPE_COLOR_RAW_YUV;
		f_bytes    = 2;
	}
	else if (p_high_res)
	{
		f_img_res  = NUI_IMAGE_RESOLUTION_1280x960;
		m_private->m_color_width  = 1280;
		m_private->m_color_height = 960;
	}

	if (m_private->m_sensor)
	{
		f_result = m_private->m_sensor->NuiImageStreamOpen(	f_img_type, f_img_res,
															0, 2,
															m_private->m_sensor_data_event,
															&m_private->m_sensor_color_stream);

		m_private->m_color_data.resize(m_private->m_color_width * m_private->m_color_height * f_bytes);

		m_private->m_nui_color_type			= f_img_type;
		m_private->m_nui_color_resolution	= f_img_res;
	}

	return SUCCEEDED (f_result);
}

bool DeviceKinect::init_depth_stream()
{
	// enable the depth stream
	HRESULT f_result = m_private->m_sensor->NuiImageStreamOpen(	NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
																NUI_IMAGE_RESOLUTION_320x240,
																0,
																2,
																nullptr,
																&m_private->m_sensor_depth_stream);

	if (SUCCEEDED(f_result))
	{
		m_private->m_depth_width  = 320;
		m_private->m_depth_height = 240;
		m_private->m_depth_data.resize(320 * 240);

		m_private->m_body_mask.resize(m_private->m_color_width * m_private->m_color_height);
		m_private->m_depth_points.resize(m_private->m_color_width * m_private->m_color_height);
		m_private->m_nui_depth_resolution = NUI_IMAGE_RESOLUTION_320x240;
	}

	return SUCCEEDED (f_result);
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

bool DeviceKinect::read_depth_frame()
{
	// attempt to get the depth frame
	NUI_IMAGE_FRAME f_frame;

	if (FAILED (m_private->m_sensor->NuiImageStreamGetNextFrame(m_private->m_sensor_depth_stream, 0, &f_frame)))
	{
		return false;
	}

	INuiFrameTexture *f_texture;
    NUI_LOCKED_RECT   f_locked_rect;
	BOOL			  f_near_mode;

	if (FAILED (m_private->m_sensor->NuiImageFrameGetDepthImagePixelFrameTexture(m_private->m_sensor_depth_stream, &f_frame, &f_near_mode, &f_texture)))
	{
		return false;
	}

    // lock the frame data so the Kinect knows not to modify it while we're reading it
    f_texture->LockRect(0, &f_locked_rect, nullptr, 0);

	// process it
	std::memcpy(m_private->m_depth_data.data(), static_cast<BYTE *>(f_locked_rect.pBits), f_locked_rect.size);

	// we're done with the texture so unlock it
    f_texture->UnlockRect(0);

    // release the frame
    m_private->m_sensor->NuiImageStreamReleaseFrame(m_private->m_sensor_depth_stream, &f_frame);
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

			f_result = m_private->m_kinect_lib->NuiImageGetColorPixelCoordinatesFromDepthPixel(
															NUI_IMAGE_RESOLUTION_640x480, nullptr,
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

bool DeviceKinect::build_index_mask()
{
	HRESULT f_result = m_private->m_sensor_coordinate_mapper->MapColorFrameToDepthFrame(	m_private->m_nui_color_type,
																							m_private->m_nui_color_resolution,
																							m_private->m_nui_depth_resolution,
																							m_private->m_depth_data.size(),
																							m_private->m_depth_data.data(),
																							m_private->m_depth_points.size(),
																							m_private->m_depth_points.data());

	if (FAILED (f_result))
		return false;

	std::fill(std::begin(m_private->m_body_mask), std::end(m_private->m_body_mask), 0);
	unsigned char *f_mask = m_private->m_body_mask.data();

	for (unsigned int f_idx = 0; f_idx < m_private->m_depth_points.size(); ++f_idx)
	{
		int f_depth_idx = (m_private->m_depth_points[f_idx].y * m_private->m_depth_width) + m_private->m_depth_points[f_idx].x;

		if (f_depth_idx >= 0 && static_cast<unsigned int> (f_depth_idx) < m_private->m_depth_data.size() && m_private->m_depth_data[f_depth_idx].playerIndex != 0)
			f_mask[f_idx] = 0xff;
	}

	return true;
}

} // namespace device
