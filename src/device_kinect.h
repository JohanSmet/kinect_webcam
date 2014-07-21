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
		virtual DeviceVideoResolution	video_resolution(int p_index);

		// update
		virtual bool update();

		// access to image data
		virtual bool color_data(int p_width, int p_height, int p_bpp, unsigned char *p_data);

	// helper function
	private :
		bool read_color_frame();
		bool color_data_32bpp(int p_hor_offset, int p_ver_offset, int p_width, int p_height, unsigned char *p_data);
		bool color_data_24bpp(int p_hor_offset, int p_ver_offset, int p_width, int p_height, unsigned char *p_data);

	// member variables
	public :
		std::unique_ptr<struct DeviceKinectPrivate>		m_private;
		static DeviceVideoResolution					m_video_resolutions[];

};

} // namespace motion

#endif // KW_DEVICE_KINECT_H

