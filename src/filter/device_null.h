///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	device_null.h
//
// Purpose	: 	fallback device
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#ifndef KW_DEVICE_NULL_H
#define KW_DEVICE_NULL_H

//
// required header files
//

#include "device.h"

#include <memory>

//
// class
//

namespace device {

class DeviceNull : public Device
{
	// member variables
	public :
		// construction
		DeviceNull();
		~DeviceNull();

		// connection to the device
		virtual bool connect_to_first();
		virtual bool disconnect();

		// resolutions
		virtual int						video_resolution_count();
		virtual int						video_resolution_preferred();
		virtual int						video_resolution_native();
		virtual DeviceVideoResolution	video_resolution(int p_index);
		virtual void					video_flip_output(bool p_flip);
		virtual void					video_set_resolution(DeviceVideoResolution p_devres);

		// body tracking
		virtual void					focus_set_joint(int p_joint);
		virtual bool					focus_availabe();
		virtual Point2D					focus_point();

		// green screen
		virtual void				  green_screen_enable(bool p_enable);

		// update
		virtual bool update();

		// access to image data
		virtual bool color_data(int p_hor_focus, int p_ver_focus, int p_width, int p_height, int p_bpp, unsigned char *p_data);

	// helper function
	private :
		bool init_color_frame();

	// member variables
	public :
		std::unique_ptr<struct DeviceNullPrivate>	m_private;
};

} // namespace motion

#endif // KW_DEVICE_NULL_H
