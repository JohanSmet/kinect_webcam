///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	device_kinect_v2.cpp
//
// Purpose	: 	interface to the kinect v2 sensor
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory 
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#include "device_kinect_v2.h"
#include "kinect_v2_wrapper.h"

#include <vector>

#include "image.h"
#include "com_utils.h"

namespace device {

struct DeviceKinectV2Private 
{
	Kinect2Funcs *					m_kinect_lib;
	IKinectSensor *					m_sensor;
	IColorFrameReader *				m_sensor_color_reader;
	IMultiSourceFrameReader	*		m_sensor_multi_reader;
	ICoordinateMapper *				m_sensor_coordinate_mapper;

	int								m_color_width;
	int								m_color_height;
	std::vector<BYTE>				m_color_data;
	DevicePixelFormat				m_color_format;

	bool							m_flip_output;
	bool							m_green_screen;

	int								m_depth_width;
	int								m_depth_height;
	std::vector<UINT16>				m_depth_data;
	std::vector<BYTE>				m_body_index_data;

	std::vector<DepthSpacePoint>	m_depth_points;
	std::vector<unsigned char>		m_body_mask;

	static const int				MAX_BODIES = 6;
	IBody *							m_kinect_bodies[MAX_BODIES];
	int								m_focus_joint;
	bool							m_focus_available;
	Point2D							m_focus;

	bool							m_reconnect;
};

HRESULT kinectv2_init_color_image(IColorFrameSource *p_source, DeviceKinectV2Private *p_private)
{
	com_safe_ptr_t<IFrameDescription>	f_frame_desc = nullptr;

	HRESULT f_result = p_source->get_FrameDescription(&f_frame_desc);
			
	if (SUCCEEDED(f_result)) f_result = f_frame_desc->get_Width(&p_private->m_color_width);
	if (SUCCEEDED(f_result)) f_result = f_frame_desc->get_Height(&p_private->m_color_height);

	if (SUCCEEDED(f_result))
	{
		p_private->m_color_data.resize(p_private->m_color_width * p_private->m_color_height * 4);
	}

	return f_result;
} 

HRESULT kinectv2_init_depth_image(IDepthFrameSource *p_source, DeviceKinectV2Private *p_private)
{
	com_safe_ptr_t<IFrameDescription>	f_frame_desc;

	HRESULT f_result = p_source->get_FrameDescription(&f_frame_desc);
			
	if (SUCCEEDED(f_result)) f_result = f_frame_desc->get_Width(&p_private->m_depth_width);
	if (SUCCEEDED(f_result)) f_result = f_frame_desc->get_Height(&p_private->m_depth_height);

	if (SUCCEEDED(f_result))
	{
		p_private->m_depth_data.resize(p_private->m_depth_width * p_private->m_depth_height);
	}

	return f_result;
}

HRESULT kinectv2_init_mask(DeviceKinectV2Private *p_private)
{
	p_private->m_depth_points.resize(p_private->m_color_width * p_private->m_color_height);
	p_private->m_body_mask.resize(p_private->m_color_width * p_private->m_color_height);

	return S_OK;
}

//
// construction
//

DeviceKinectV2::DeviceKinectV2() :	m_private(std::make_unique<DeviceKinectV2Private>())
{
	m_private->m_sensor						= nullptr;
	m_private->m_sensor_color_reader		= nullptr;
	m_private->m_sensor_multi_reader		= nullptr;
	m_private->m_sensor_coordinate_mapper	= nullptr;
	m_private->m_color_format				= DPF_RGBA;
	m_private->m_flip_output				= true;
	m_private->m_green_screen				= false;
	m_private->m_reconnect					= false;
}

DeviceKinectV2::~DeviceKinectV2()
{
}

//
// connection to the device
//

bool DeviceKinectV2::connect_to_first()
{
	m_private->m_sensor						= nullptr;
	m_private->m_sensor_color_reader		= nullptr;
	m_private->m_sensor_multi_reader		= nullptr;
	m_private->m_sensor_coordinate_mapper	= nullptr;

	std::fill(std::begin(m_private->m_kinect_bodies), std::end(m_private->m_kinect_bodies), nullptr);

	// try to load the kinect library
	m_private->m_kinect_lib = kinect_v2_load_library();

	if (m_private->m_kinect_lib == nullptr)
	{
		return false;
	}

	// connect to the default sensor
	HRESULT f_result = m_private->m_kinect_lib->GetDefaultKinectSensor(&m_private->m_sensor);

	if (FAILED(f_result) || !m_private->m_sensor)
	{
		return false;
	}

	// initialize the kinect
	if (SUCCEEDED(f_result))
	{
		f_result = m_private->m_sensor->Open();
	}

	// wait for the sensor to become available (300ms was quoted by ms on the kinect forum, but gave unreliable results on my machine)
	if (SUCCEEDED(f_result) && !m_private->m_reconnect)
	{
		WAITABLE_HANDLE	f_sensor_waitable = 0;	
		f_result = m_private->m_sensor->SubscribeIsAvailableChanged(&f_sensor_waitable);

		if (SUCCEEDED(f_result))
		{
			if (WaitForSingleObject(reinterpret_cast<HANDLE> (f_sensor_waitable), 1000) != WAIT_OBJECT_0)
				f_result = E_ABORT;

			if (SUCCEEDED(f_result))
			{
				BOOLEAN		f_available = false;
				f_result = m_private->m_sensor->get_IsAvailable(&f_available);

				if (SUCCEEDED(f_result) && !f_available)
					f_result = E_ABORT;
			}

			m_private->m_sensor->UnsubscribeIsAvailableChanged(f_sensor_waitable);
			m_private->m_reconnect = true;
		}
	}

	// obtain a color reader (seperate because framerate may vary)
	if (SUCCEEDED(f_result))
	{
		com_safe_ptr_t<IColorFrameSource> f_color_frame_source;
 
		f_result = m_private->m_sensor->get_ColorFrameSource(&f_color_frame_source);

        if (SUCCEEDED(f_result))
        {
            f_result = f_color_frame_source->OpenReader(&m_private->m_sensor_color_reader);
        }

		if (SUCCEEDED(f_result))
		{
			f_result = kinectv2_init_color_image(f_color_frame_source.get(), m_private.get());
		}
	}

	// obtain a multisource-reader for the other sources
	if (SUCCEEDED(f_result))
	{
		f_result = m_private->m_sensor->OpenMultiSourceFrameReader(	FrameSourceTypes_BodyIndex | FrameSourceTypes_Body | FrameSourceTypes_Depth,
																	&m_private->m_sensor_multi_reader);
	}

	// initialization for the depth reader
	if (SUCCEEDED(f_result))
	{
		com_safe_ptr_t<IDepthFrameSource> f_depth_frame_source;
 
		f_result = m_private->m_sensor->get_DepthFrameSource(&f_depth_frame_source);

		if (SUCCEEDED(f_result))
		{
			f_result = kinectv2_init_depth_image(f_depth_frame_source.get(), m_private.get());
		}
	}

	// initialization for the body index reader
	if (SUCCEEDED(f_result)) 
	{
		// dimensions are the same as the depth buffer
		m_private->m_body_index_data.resize(m_private->m_depth_width * m_private->m_depth_height);

		if (SUCCEEDED(f_result))
		{
			f_result = kinectv2_init_mask(m_private.get());
		}
	}

	// obtain a coordinate mapper
	if (SUCCEEDED(f_result))
	{
		f_result = m_private->m_sensor->get_CoordinateMapper(&m_private->m_sensor_coordinate_mapper);
	}
	
	// release resources if something failed
	if (FAILED(f_result))
	{
		com_safe_release(&m_private->m_sensor_coordinate_mapper);
		com_safe_release(&m_private->m_sensor_multi_reader);
		com_safe_release(&m_private->m_sensor_color_reader);
		com_safe_release(&m_private->m_sensor);
	}

	if (m_private->m_sensor != nullptr)
	{	
		m_private->m_focus_joint	 = JointType_Head;
		m_private->m_focus_available = false;
		m_private->m_focus			 = {0, 0};
		return true;
	}
    
	return false;
}

bool DeviceKinectV2::disconnect()
{
	com_safe_release(&m_private->m_sensor_color_reader);
	
	if (m_private->m_sensor)
	{
		m_private->m_sensor->Close();
		com_safe_release(&m_private->m_sensor);
	}

	m_private->m_color_data.clear();
	m_private->m_depth_data.clear();
	m_private->m_body_index_data.clear();

	return true;
}

//
// video resolutions
//

DeviceVideoResolution DeviceKinectV2::m_video_resolutions[] = { { 320,  240, 32, 30, DPF_RGBA},
																{ 640,  480, 32, 30, DPF_RGBA},
																{1920, 1080, 32, 30, DPF_RGBA},
																{ 320,  240, 24, 30, DPF_RGB},
																{ 640,  480, 24, 30, DPF_RGB},
																{1920, 1080, 24, 30, DPF_RGB},
																{ 320,  240, 16, 30, DPF_YUY2},		// XXX do not assume the native format is YUY2
																{1920, 1080, 16, 30, DPF_YUY2}
															  };

int	DeviceKinectV2::video_resolution_count()
{
	return sizeof(m_video_resolutions) / sizeof(DeviceVideoResolution);
}

int	DeviceKinectV2::video_resolution_preferred()
{
	return 0;
}

int	DeviceKinectV2::video_resolution_native()
{
	return 2;
}

DeviceVideoResolution DeviceKinectV2::video_resolution(int p_index)
{
	return m_video_resolutions[p_index];
} 

void DeviceKinectV2::video_set_resolution(DeviceVideoResolution p_devres)
{
	m_private->m_color_format = p_devres.m_pixel_format;
}

void DeviceKinectV2::video_flip_output(bool p_flip)
{
	m_private->m_flip_output = p_flip;
}

//
// body tracking
//

void DeviceKinectV2::focus_set_joint(int p_joint)
{
	if (p_joint >= 0 && p_joint < JointType_Count)
		m_private->m_focus_joint = p_joint;
}

bool DeviceKinectV2::focus_availabe()
{
	return m_private->m_focus_available;
}

Point2D	DeviceKinectV2::focus_point()
{
	return m_private->m_focus;
}

//
// green screen
//

void DeviceKinectV2::green_screen_enable(bool p_enable)
{
	m_private->m_green_screen = p_enable;
}

//
// update detected data
//

bool DeviceKinectV2::update()
{
	// sensor connected ?
	if (!m_private->m_sensor)
		return false;

	// read the color frame separately - the kinect can drop to 15fps in low light conditions 
	//	but we don't want to delay the other data sources
	bool f_new_data = read_color_frame();

	// check if there's new data available in the multi-source reader
	com_safe_ptr_t<IMultiSourceFrame>	f_multi_frame = nullptr;
	if (m_private->m_sensor_multi_reader && SUCCEEDED (m_private->m_sensor_multi_reader->AcquireLatestFrame(&f_multi_frame)))
	{ 
		f_new_data |= read_body_index_frame(f_multi_frame.get());
		f_new_data |= read_body_frame(f_multi_frame.get());
		f_new_data |= read_depth_frame(f_multi_frame.get());
	}

	return f_new_data;

}

//
// access to image data
//

bool DeviceKinectV2::color_data(int p_hor_focus, int p_ver_focus, int p_width, int p_height, int p_bpp, unsigned char *p_data)
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

	switch (m_private->m_color_format)
	{	
		case DPF_RGBA :
			if (m_private->m_green_screen)
				return img::copy_region_32bpp_32bpp_mask(	m_private->m_color_width, m_private->m_color_height, m_private->m_color_data.data(), m_private->m_body_mask.data(),
															f_hor_offset, f_ver_offset, p_width, p_height, p_data,
															m_private->m_flip_output);
			else 
				return img::copy_region_32bpp_32bpp(m_private->m_color_width, m_private->m_color_height, m_private->m_color_data.data(),
													f_hor_offset, f_ver_offset, p_width, p_height, p_data,
													m_private->m_flip_output);
		case DPF_RGB :
			if (m_private->m_green_screen)
				return img::copy_region_32bpp_24bpp_mask(	m_private->m_color_width, m_private->m_color_height, m_private->m_color_data.data(), m_private->m_body_mask.data(),
															f_hor_offset, f_ver_offset, p_width, p_height, p_data,
															m_private->m_flip_output);
			else
				return img::copy_region_32bpp_24bpp(m_private->m_color_width, m_private->m_color_height, m_private->m_color_data.data(),
													f_hor_offset, f_ver_offset, p_width, p_height, p_data,
													m_private->m_flip_output);

		case DPF_YUY2 :
			return img::copy_region_yuy2(	m_private->m_color_width, m_private->m_color_height, m_private->m_color_data.data(),
											f_hor_offset, f_ver_offset, p_width, p_height, p_data);
	
		default :
			return false;
	}
}

bool DeviceKinectV2::read_color_frame()
{
	if (!m_private->m_sensor_color_reader)
		return false;

	com_safe_ptr_t<IColorFrame> f_frame = nullptr;

	// try to read the next frame
	auto f_result = m_private->m_sensor_color_reader->AcquireLatestFrame(&f_frame);

	if (SUCCEEDED(f_result) && (m_private->m_color_format == DPF_RGB || m_private->m_color_format == DPF_RGBA))
	{
		f_result = f_frame->CopyConvertedFrameDataToArray(static_cast<UINT> (m_private->m_color_data.size()), m_private->m_color_data.data(), ColorImageFormat_Bgra);
	}

	if (SUCCEEDED(f_result) && m_private->m_color_format == DPF_YUY2)
	{
		f_result = f_frame->CopyRawFrameDataToArray(static_cast<UINT> (m_private->m_color_data.size() / 2), m_private->m_color_data.data());
	}

	return SUCCEEDED(f_result);
}

bool DeviceKinectV2::read_body_index_frame(IMultiSourceFrame *p_multi_source_frame)
{
	com_safe_ptr_t<IBodyIndexFrameReference> f_frame_ref = nullptr;
	com_safe_ptr_t<IBodyIndexFrame>			 f_frame = nullptr;

	if (!p_multi_source_frame)
		return false;

	// try to read the next frame
	auto f_result = p_multi_source_frame->get_BodyIndexFrameReference(&f_frame_ref);

	if (SUCCEEDED(f_result))
	{	
		f_result = f_frame_ref->AcquireFrame(&f_frame);
	}

	if (SUCCEEDED(f_result))
	{
		f_result = f_frame->CopyFrameDataToArray(static_cast<UINT> (m_private->m_body_index_data.size()), reinterpret_cast<BYTE *> (m_private->m_body_index_data.data()));
	}

	return SUCCEEDED(f_result);
}

bool DeviceKinectV2::read_body_frame(IMultiSourceFrame *p_multi_source_frame)
{
	com_safe_ptr_t<IBodyFrameReference> f_frame_ref = nullptr;
	com_safe_ptr_t<IBodyFrame>			f_frame = nullptr;

	if (!p_multi_source_frame)
		return false;

	// try and read the next frame
	auto f_result = p_multi_source_frame->get_BodyFrameReference(&f_frame_ref);

	if (SUCCEEDED(f_result))
	{
		f_result = f_frame_ref->AcquireFrame(&f_frame);
	}	

	if (SUCCEEDED(f_result))
	{
		f_result = f_frame->GetAndRefreshBodyData(DeviceKinectV2Private::MAX_BODIES, m_private->m_kinect_bodies);
	}

	// iterate of the bodies
	m_private->m_focus_available = false;

	for (auto f_idx = 0; SUCCEEDED(f_result) && !m_private->m_focus_available && f_idx < DeviceKinectV2Private::MAX_BODIES; ++f_idx)
	{
		auto *  f_body		 = m_private->m_kinect_bodies[f_idx];
		BOOLEAN f_is_tracked = false;
		Joint	f_joints[JointType_Count];

		// is the body tracked ?
		if (SUCCEEDED(f_result))
		{
			f_result = f_body->get_IsTracked(&f_is_tracked);
		}

		// retrieve information about the joints of the tracked body
		if (SUCCEEDED(f_result) && f_is_tracked)
		{
			f_result = f_body->GetJoints(JointType_Count, f_joints);
		}
	
		// convert the location of the focus joint to color space
		if (SUCCEEDED(f_result) && f_is_tracked)
		{
			ColorSpacePoint	f_point;
			f_result = m_private->m_sensor_coordinate_mapper->MapCameraPointToColorSpace(f_joints[m_private->m_focus_joint].Position, &f_point);

			if (SUCCEEDED (f_result))
			{
				m_private->m_focus_available = true;
				m_private->m_focus.m_x		 = static_cast<int> (f_point.X);
				m_private->m_focus.m_y		 = static_cast<int> (f_point.Y);
			}
		}
	}

	return SUCCEEDED(f_result);
}

bool DeviceKinectV2::read_depth_frame(IMultiSourceFrame *p_multi_source_frame)
{
	com_safe_ptr_t<IDepthFrameReference> f_frame_ref = nullptr;
	com_safe_ptr_t<IDepthFrame>			 f_frame = nullptr;

	if (!p_multi_source_frame)
		return false;

	// try to read the next frame
	auto f_result = p_multi_source_frame->get_DepthFrameReference(&f_frame_ref);

	if (SUCCEEDED(f_result))
	{	
		f_result = f_frame_ref->AcquireFrame(&f_frame);
	}

	if (SUCCEEDED(f_result))
	{
		f_result = f_frame->CopyFrameDataToArray(static_cast<UINT> (m_private->m_body_index_data.size()), reinterpret_cast<UINT16 *> (m_private->m_depth_data.data()));
	}

	return SUCCEEDED(f_result);
}

bool DeviceKinectV2::copy_index_buffer(int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data)
{
	const int	f_s_pixel_size	= 1;
	const int	f_d_pixel_size	= 4;
	const int	f_d_line_size	= p_dst_width * f_d_pixel_size;
	const int	f_s_line_stride	= m_private->m_depth_width * f_s_pixel_size;

	p_dst_x = 0;
	p_dst_y = 0;

	const auto *f_src_start		= m_private->m_body_index_data.data() + (p_dst_x * f_s_pixel_size) + (p_dst_y * f_s_line_stride);

	// swap the picture vertically
	for (const auto *f_src_line = f_src_start + (f_s_line_stride * (p_dst_height - 1));	// start of the last line
		 f_src_line >= f_src_start;
		 f_src_line -= f_s_line_stride, p_dst_data += f_d_line_size)
	{
		auto *f_src = f_src_line;
		auto *f_dst = p_dst_data;

		for (int f_w = 0; f_w < p_dst_width; ++f_w)
		{
			unsigned char f_color =  (*f_src++ == 0xff) ? 0x00 : 0xff;
			
			*f_dst++ = f_color;
			*f_dst++ = f_color;
			*f_dst++ = f_color;
			*f_dst++ = 0;
		}
	}

	return true;
}

bool DeviceKinectV2::build_index_mask()
{
	HRESULT f_result = m_private->m_sensor_coordinate_mapper->MapColorFrameToDepthSpace( m_private->m_depth_width * m_private->m_depth_height,
																						 m_private->m_depth_data.data(),
																						 m_private->m_color_width * m_private->m_color_height,
																						 m_private->m_depth_points.data());

	if (FAILED (f_result))
		return false;
	
	std::fill(std::begin(m_private->m_body_mask), std::end(m_private->m_body_mask), 0);
	unsigned char *f_mask = m_private->m_body_mask.data();

	for (unsigned int f_idx = 0; f_idx < m_private->m_depth_points.size(); ++f_idx)
	{
		int f_depth_idx = (static_cast<int>(m_private->m_depth_points[f_idx].Y) * m_private->m_depth_width) + static_cast<int> (m_private->m_depth_points[f_idx].X);
		
		if (f_depth_idx >= 0 && static_cast<unsigned int> (f_depth_idx) < m_private->m_depth_data.size() && m_private->m_body_index_data[f_depth_idx] != 0xff)
			f_mask[f_idx] = 0xff;
	}

	return true;
}

} // namespace device