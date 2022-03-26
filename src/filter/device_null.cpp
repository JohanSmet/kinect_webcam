///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	device_null
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

#include "device_null.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <streams.h>

#include <vector>

namespace device {

struct DeviceNullPrivate
{
	DeviceVideoResolution	m_resolution;
	std::vector<BYTE>		m_color_data;
};

//
// construction
//

DeviceNull::DeviceNull() :	m_private(std::make_unique<DeviceNullPrivate>())
{
	m_private->m_resolution.m_width				= 320;
	m_private->m_resolution.m_height			= 240;
	m_private->m_resolution.m_bits_per_pixel	= 24;
	m_private->m_resolution.m_framerate			= 10;
	m_private->m_resolution.m_pixel_format		= DPF_RGB;

	m_private->m_color_data.resize(320 * 240 * 3);
}

DeviceNull::~DeviceNull()
{
}

//
// connection to the device
//

bool DeviceNull::connect_to_first()
{
	return init_color_frame();
}

bool DeviceNull::disconnect()
{
	return true;
}

//
// video resolutions
//

int	DeviceNull::video_resolution_count()
{
	return 1;
}

int	DeviceNull::video_resolution_preferred()
{
	return 0;
}

int	DeviceNull::video_resolution_native()
{
	return 0;
}

DeviceVideoResolution DeviceNull::video_resolution(int p_index)
{
	return m_private->m_resolution;
}

void DeviceNull::video_set_resolution(DeviceVideoResolution p_devres)
{
}

void DeviceNull::video_flip_output(bool p_flip)
{
}

//
// body tracking
//


void DeviceNull::focus_set_joint(int p_joint)
{
}

bool DeviceNull::focus_availabe()
{
	return false;
}

Point2D	DeviceNull::focus_point()
{
	return {0,0};
}

//
// green screen
//

void DeviceNull::green_screen_enable(bool p_enable)
{
}

//
// update detected data
//

bool DeviceNull::update()
{
	return true;
}

//
// access to image data
//

bool DeviceNull::color_data(int p_hor_focus, int p_ver_focus, int p_width, int p_height, int p_bpp, unsigned char *p_data)
{
	if (m_private->m_resolution.m_width				!= p_width ||
		m_private->m_resolution.m_height			!= p_height ||
		m_private->m_resolution.m_bits_per_pixel	!= p_bpp)
	{
		return false;
	}

	std::memcpy(p_data, m_private->m_color_data.data(), m_private->m_color_data.size());
	return true;
}

bool DeviceNull::init_color_frame()
{
	BITMAPINFO	f_bmi		= {0};
	void *		f_buffer	= nullptr;

	// create a device independent bitmap of the correct format
	f_bmi.bmiHeader.biCompression    = BI_RGB;
	f_bmi.bmiHeader.biBitCount       = m_private->m_resolution.m_bits_per_pixel;
    f_bmi.bmiHeader.biSize           = sizeof(BITMAPINFOHEADER);
    f_bmi.bmiHeader.biWidth          = m_private->m_resolution.m_width;
    f_bmi.bmiHeader.biHeight         = m_private->m_resolution.m_height;
    f_bmi.bmiHeader.biPlanes         = 1;
    f_bmi.bmiHeader.biSizeImage      = GetBitmapSize(&f_bmi.bmiHeader);
    f_bmi.bmiHeader.biClrImportant   = 0;

	auto f_dib = CreateDIBSection(nullptr, &f_bmi, DIB_RGB_COLORS, &f_buffer, nullptr, 0);

	if (!f_dib)
		return false;

	// attach the DIB to a device context
	auto f_hdc = GetDC(nullptr);
	auto f_paintdc = CreateCompatibleDC(f_hdc);
	SetMapMode(f_paintdc, GetMapMode(f_hdc));
	SelectObject(f_paintdc, f_dib);

	// text properties
	SetBkColor  (f_paintdc, RGB (0,0,0));
	SetTextColor(f_paintdc, RGB(255,255,255));
	SetTextAlign(f_paintdc, TA_CENTER);

	// write the text to the DIB
	const wchar_t *f_msg = L"No Kinect available.";
	TextOut(f_paintdc, 160, 110, f_msg, wcslen(f_msg));

	// save the buffer
	memcpy(m_private->m_color_data.data(), f_buffer, m_private->m_color_data.size());

	// cleanup
	DeleteDC(f_paintdc);

	return true;
}


} // namespace device
