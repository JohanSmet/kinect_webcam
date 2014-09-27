///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	image.h
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

#ifndef KW_IMAGE_H
#define KW_IMAGE_H

namespace img {

bool copy_region_32bpp_32bpp(int p_src_width, int p_src_height, unsigned char *p_src_data,
							 int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data);

bool copy_region_32bpp_32bpp_flipped(int p_src_width, int p_src_height, unsigned char *p_src_data,
							 int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data);

bool copy_region_32bpp_24bpp_flipped(int p_src_width, int p_src_height, unsigned char *p_src_data,
							 int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data);

bool copy_region_32bpp_24bpp(int p_src_width, int p_src_height, unsigned char *p_src_data,
							 int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data);

bool copy_region_yuy2(int p_src_width, int p_src_height, unsigned char *p_src_data,
					  int p_dst_x, int p_dst_y, int p_dst_width, int p_dst_height, unsigned char *p_dst_data);

} // namespace img

#endif // KW_IMAGE_H