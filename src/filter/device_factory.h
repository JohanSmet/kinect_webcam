///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	input/motion/device_factory.h
//
// Purpose	: 	create motion input devices
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#ifndef KW_DEVICE_FACTORY_H
#define KW_DEVICE_FACTORY_H

#include <string>
#include <memory>

namespace device {

std::unique_ptr<class Device> device_factory(const std::string &p_type);

} // namespace motion

#endif // KW_DEVICE_FACTORY_H

