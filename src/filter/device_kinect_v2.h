///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	device_kinect_v2.h
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

#ifndef KW_DEVICE_KINECT_V2_H
#define KW_DEVICE_KINECT_V2_H

//
// required header files
//

#include "device.h"

#include <memory>

//
// class
//

struct IMultiSourceFrame;

namespace device {

class DeviceKinectV2 : public Device
{
	// member variables
	public :
		// construction
		DeviceKinectV2();
		~DeviceKinectV2();

		// connection to the device
		virtual bool connect_to_first();
		virtual bool disconnect();

		// resolutions
		virtual int						video_resolution_count();
		virtual int						video_resolution_preferred();
		virtual int						video_resolution_native();
		virtual DeviceVideoResolution	video_resolution(int p_index);

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
		bool read_color_frame();
		bool read_body_index_frame(IMultiSourceFrame *p_multi_source_frame);
		bool read_body_frame(IMultiSourceFrame *p_multi_source_frame);

	// member variables
	public :
		std::unique_ptr<struct DeviceKinectV2Private>	m_private;
		static DeviceVideoResolution					m_video_resolutions[];
};

} // namespace motion

#endif // KW_DEVICE_KINECT_V2_H

