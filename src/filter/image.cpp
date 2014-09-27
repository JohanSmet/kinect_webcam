///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	image.cpp
//
// Purpose	: 	image manipulation functions
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory 
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#include "image.h"
#include <memory>

namespace img {

bool copy_region_32bpp_32bpp(int p_src_width, int p_src_height, unsigned char *p_src_data,
							 int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data)
{
	const int	f_pixel_size	= 4;
	int			f_line_size		= p_dst_width * f_pixel_size;
	int			f_line_stride	= p_src_width * f_pixel_size;
	const auto *f_src_start		= p_src_data + (p_dst_x * f_pixel_size) + (p_dst_y * f_line_stride);
	const auto *f_dst_end		= p_dst_data + (f_line_size * p_dst_height);

	// swap the picture vertically
	for (const auto *f_src_line = p_src_data + (p_dst_x * f_pixel_size) + (p_dst_y * f_line_stride);
		 p_dst_data < f_dst_end;
		 f_src_line += f_line_stride, p_dst_data += f_line_size)
	{
		memcpy(p_dst_data, f_src_line, f_line_size);
	}
	
	return true;
}

bool copy_region_32bpp_32bpp_flipped(int p_src_width, int p_src_height, unsigned char *p_src_data,
									 int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data)
{
	const int	f_pixel_size	= 4;
	int			f_line_size		= p_dst_width * f_pixel_size;
	int			f_line_stride	= p_src_width * f_pixel_size;
	const auto *f_src_start		= p_src_data + (p_dst_x * f_pixel_size) + (p_dst_y * f_line_stride);

	// swap the picture vertically
	for (const auto *f_src_line = f_src_start + (f_line_stride * (p_dst_height - 1));	// start of the last line
		 f_src_line >= f_src_start;
		 f_src_line -= f_line_stride, p_dst_data += f_line_size)
	{
		memcpy(p_dst_data, f_src_line, f_line_size);
	}
	
	return true;
}
bool copy_region_32bpp_24bpp(int p_src_width, int p_src_height, unsigned char *p_src_data,
							 int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data)
{
	// XXX some optimization of this routine wouldn't be a bad idea ;-)
	const int	f_s_pixel_size	= 4;
	const int	f_d_pixel_size	= 3;
	const int	f_d_line_size	= p_dst_width * f_d_pixel_size;
	const int	f_s_line_stride	= p_src_width * f_s_pixel_size;

	const auto *f_dst_end		= p_dst_data + (f_d_line_size * p_dst_height);

	// swap the picture vertically
	for (const auto *f_src_line = p_src_data + (p_dst_x * f_s_pixel_size) + (p_dst_y * f_s_line_stride);
		 p_dst_data < f_dst_end;
		 f_src_line += f_s_line_stride, p_dst_data += f_d_line_size)
	{
		auto *f_src = f_src_line;
		auto *f_dst = p_dst_data;

		for (int f_w = 0; f_w < p_dst_width; ++f_w)
		{
			*f_dst++ = *f_src++;
			*f_dst++ = *f_src++;
			*f_dst++ = *f_src++;
			++f_src;
		}
	}

	return true;
}

bool copy_region_32bpp_24bpp_flipped(int p_src_width, int p_src_height, unsigned char *p_src_data,
									 int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data)
{
	// XXX some optimization of this routine wouldn't be a bad idea ;-)
	const int	f_s_pixel_size	= 4;
	const int	f_d_pixel_size	= 3;
	const int	f_d_line_size	= p_dst_width * f_d_pixel_size;
	const int	f_s_line_stride	= p_src_width * f_s_pixel_size;

	const auto *f_src_start		= p_src_data + (p_dst_x * f_s_pixel_size) + (p_dst_y * f_s_line_stride);

	// swap the picture vertically
	for (const auto *f_src_line = f_src_start + (f_s_line_stride * (p_dst_height - 1));	// start of the last line
		 f_src_line >= f_src_start;
		 f_src_line -= f_s_line_stride, p_dst_data += f_d_line_size)
	{
		auto *f_src = f_src_line;
		auto *f_dst = p_dst_data;

		for (int f_w = 0; f_w < p_dst_width; ++f_w)
		{
			*f_dst++ = *f_src++;
			*f_dst++ = *f_src++;
			*f_dst++ = *f_src++;
			++f_src;
		}
	}

	return true;
}

bool copy_region_yuy2(int p_src_width, int p_src_height, unsigned char *p_src_data,
					  int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data)
{
	const int	f_pixel_size	= 2;
	int			f_line_size		= p_dst_width * f_pixel_size;
	int			f_line_stride	= p_src_width * f_pixel_size;
	const auto *f_dst_end		= p_dst_data + (f_line_size * p_dst_height);

	for (const auto *f_src_line = p_src_data + (p_dst_x * f_pixel_size) + (p_dst_y * f_line_stride);
		 p_dst_data < f_dst_end;
		 f_src_line += f_line_stride, p_dst_data += f_line_size)
	{
		memcpy(p_dst_data, f_src_line, f_line_size);
	}
	
	return true;
}

} // namespace img
