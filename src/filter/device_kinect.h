///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	device_kinect.h
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


#ifndef KW_DEVICE_KINECT_H
#define KW_DEVICE_KINECT_H

//
// required header files
//

#include "device.h"

#include <memory>

//
// class
//

namespace device {

class DeviceKinect : public Device
{
	// member variables
	public :
		// construction
		DeviceKinect();
		~DeviceKinect();

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
		virtual void				  focus_set_joint(int p_joint);
		virtual bool				  focus_availabe();
		virtual Point2D				  focus_point();

		// update
		virtual bool update();

		// access to image data
		virtual bool color_data(int p_hor_focus, int p_ver_focus, int p_width, int p_height, int p_bpp, unsigned char *p_data);

	// helper function
	private :
		bool init_color_stream(DevicePixelFormat p_format, bool p_high_res);
		bool read_color_frame();
		bool read_skeleton_frame();

	// member variables
	public :
		std::unique_ptr<struct DeviceKinectPrivate>		m_private;
		static DeviceVideoResolution					m_video_resolutions[];

};

} // namespace motion

#endif // KW_DEVICE_KINECT_H

