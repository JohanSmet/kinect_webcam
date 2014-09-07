///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	settings.h
//
// Purpose	: 	persistent settings
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory 
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#ifndef KW_SETTINGS_H
#define KW_SETTINGS_H

#include <string>

namespace settings {

// interface functions
void load();
void save();
void cleanup();

bool have_changed();

// some fun with macro's to declare the settings
#if !defined (KW_SETTINGS_IMPLEMENATION)

#undef  SETTING_BOOLEAN
#define SETTING_BOOLEAN(p_name, p_default)	extern bool p_name;

#undef  SETTING_INTEGER
#define SETTING_INTEGER(p_name, p_default)	extern int p_name;

#undef  SETTING_STRING
#define SETTING_STRING(p_name, p_default)	extern std::wstring p_name;

#else

#undef  SETTING_BOOLEAN
#define SETTING_BOOLEAN(p_name, p_default)	bool p_name = p_default;

#undef  SETTING_INTEGER
#define SETTING_INTEGER(p_name, p_default)	int p_name = p_default;

#undef  SETTING_STRING
#define SETTING_STRING(p_name, p_default)	std::wstring p_name = p_default;

#endif // KW_SETTINGS_IMPLEMENTATION

#include "settings_list.h"


} // namespace settings


#endif // KW_SETTINGS_H
