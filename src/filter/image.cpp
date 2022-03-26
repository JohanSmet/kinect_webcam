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

#include <opencv2/opencv.hpp>

namespace img {

bool copy_region_32bpp_32bpp(int p_src_width, int p_src_height, unsigned char *p_src_data,
							 int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data,
							 bool p_flip)
{
	cv::Mat f_src(p_src_height, p_src_width, CV_8UC4, p_src_data);
	cv::Mat f_dst(p_dst_height, p_dst_width, CV_8UC4, p_dst_data);

	// cropping
	cv::Mat f_src_cropped(f_src, cv::Rect(p_dst_x, p_dst_y, p_dst_width, p_dst_height));

	// copy to output (flipped or not)
	if (p_flip)
		cv::flip(f_src_cropped, f_dst, 0);
	else
		f_src_cropped.copyTo(f_dst);

	return true;
}

bool copy_region_32bpp_32bpp_mask(	int p_src_width, int p_src_height, unsigned char *p_src_data, unsigned char *p_mask_channel,
									int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data,
									bool p_flip)
{
	cv::Mat f_src(p_src_height, p_src_width, CV_8UC4, p_src_data);
	cv::Mat f_dst(p_dst_height, p_dst_width, CV_8UC4, p_dst_data);
	cv::Mat f_msk(p_src_height, p_src_width, CV_8UC1, p_mask_channel);

	// cropping
	cv::Mat f_src_cropped(f_src, cv::Rect(p_dst_x, p_dst_y, p_dst_width, p_dst_height));
	cv::Mat f_msk_cropped(f_msk, cv::Rect(p_dst_x, p_dst_y, p_dst_width, p_dst_height));

	if (p_flip)
	{
		// mask to a new matrix
		cv::Mat f_masked;
		f_src_cropped.copyTo(f_masked, f_msk_cropped);

		// flip and copy masked temporary to destination
		cv::flip(f_masked, f_dst, 0);
	}
	else
	{
		// mask to the destination
		f_src_cropped.copyTo(f_dst, f_msk_cropped);
	}

	return true;
}

bool copy_region_32bpp_24bpp(int p_src_width, int p_src_height, unsigned char *p_src_data,
							 int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data,
							 bool p_flip)
{
	cv::Mat f_src(p_src_height, p_src_width, CV_8UC4, p_src_data);
	cv::Mat f_dst(p_dst_height, p_dst_width, CV_8UC3, p_dst_data);

	// cropping
	cv::Mat f_src_cropped(f_src, cv::Rect(p_dst_x, p_dst_y, p_dst_width, p_dst_height));

	// color space conversion
	cv::Mat f_src_rgb;

	cv::cvtColor(f_src_cropped, f_src_rgb, cv::COLOR_RGBA2RGB);

	// copy to output (flipped or not)
	if (p_flip)
		cv::flip(f_src_rgb, f_dst, 0);
	else
		f_src_rgb.copyTo(f_dst);

	return true;
}

bool copy_region_32bpp_24bpp_mask(	int p_src_width, int p_src_height, unsigned char *p_src_data, unsigned char *p_mask_channel,
									int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data,
									bool p_flip)
{
	cv::Mat f_src(p_src_height, p_src_width, CV_8UC4, p_src_data);
	cv::Mat f_dst(p_dst_height, p_dst_width, CV_8UC4, p_dst_data);
	cv::Mat f_msk(p_src_height, p_src_width, CV_8UC1, p_mask_channel);

	// cropping
	cv::Mat f_src_cropped(f_src, cv::Rect(p_dst_x, p_dst_y, p_dst_width, p_dst_height));
	cv::Mat f_msk_cropped(f_msk, cv::Rect(p_dst_x, p_dst_y, p_dst_width, p_dst_height));

	// color space conversion
	cv::Mat f_src_rgb;

	cv::cvtColor(f_src_cropped, f_src_rgb, cv::COLOR_RGBA2RGB);

	if (p_flip)
	{
		// mask to a new matrix
		cv::Mat f_masked;
		f_src_rgb.copyTo(f_masked, f_msk_cropped);

		// flip and copy masked temporary to destination
		cv::flip(f_masked, f_dst, 0);
	}
	else
	{
		// mask to the destination
		f_src_rgb.copyTo(f_dst, f_msk_cropped);
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
