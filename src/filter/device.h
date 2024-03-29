///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	input/motion/device.h
//
// Purpose	: 	base class for motion detection devices
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#ifndef KW_DEVICE_H
#define KW_DEVICE_H

//
// interface
//

namespace device {

enum DevicePixelFormat
{
	DPF_RGB,
	DPF_RGBA,
	DPF_YUY2
};

struct DeviceVideoResolution
{
	int					m_width;
	int					m_height;
	int					m_bits_per_pixel;
	int					m_framerate;
	DevicePixelFormat	m_pixel_format;
};

struct Point2D {
	int	m_x;
	int m_y;
};

class Device
{
	public :
		// construction
		virtual ~Device() {}

		// connection to the device
		virtual bool connect_to_first() = 0;
		virtual bool disconnect() = 0;

		// resolutions
		virtual int					  video_resolution_count() = 0;
		virtual int					  video_resolution_preferred() = 0;
		virtual int					  video_resolution_native() = 0;
		virtual DeviceVideoResolution video_resolution(int p_index) = 0;
		virtual void				  video_flip_output(bool p_flip) = 0;
		virtual void				  video_set_resolution(DeviceVideoResolution p_devres) = 0;

		// body tracking
		virtual void				  focus_set_joint(int p_joint) = 0;
		virtual bool				  focus_availabe() = 0;
		virtual Point2D				  focus_point() = 0;

		// green screen
		virtual void				  green_screen_enable(bool p_enable) = 0;

		// update
		virtual bool update() = 0;

		// access to image data
		virtual bool color_data(int p_hor_focus, int p_ver_focus, int p_width, int p_height, int p_bpp, unsigned char *p_data) = 0;
};

} // namespace motion

#endif // KW_DEVICE_H

