///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	settings.cpp
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

#define KW_SETTINGS_IMPLEMENATION
#include "settings.h"

#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

namespace settings {

inline int read_integer(HKEY p_reg, const wchar_t *p_name, int p_default)
{
	DWORD f_value;
	DWORD f_size = sizeof(f_value);

	if (RegGetValue(p_reg, NULL, p_name, RRF_RT_REG_DWORD, NULL, &f_value, &f_size) != ERROR_SUCCESS)
		return p_default;

	return f_value;
}

inline bool write_integer(HKEY p_reg, const wchar_t *p_name, int p_value)
{
	DWORD f_value = p_value;
	return RegSetValueEx(p_reg, p_name, 0, REG_DWORD, reinterpret_cast<BYTE *> (&f_value), sizeof(f_value)) == ERROR_SUCCESS;
}

inline bool read_bool(HKEY p_reg, const wchar_t *p_name, bool p_default)
{
	DWORD f_value;
	DWORD f_size = sizeof(f_value);

	if (RegGetValue(p_reg, NULL, p_name, RRF_RT_REG_DWORD, NULL, &f_value, &f_size) != ERROR_SUCCESS)
		return p_default;

	return f_value > 0;
}

inline bool write_bool(HKEY p_reg, const wchar_t *p_name, bool p_value)
{
	DWORD f_value = p_value;
	return RegSetValueEx(p_reg, p_name, 0, REG_DWORD, reinterpret_cast<BYTE *> (&f_value), sizeof(f_value)) == ERROR_SUCCESS;
}

inline std::wstring read_string(HKEY p_reg, const wchar_t *p_name, std::wstring p_default)
{
	wchar_t f_buffer[1024] = L"";
	DWORD   f_size = 1023;

	if (RegGetValue(p_reg, NULL, p_name, RRF_RT_REG_SZ, NULL, f_buffer, &f_size) != ERROR_SUCCESS)
		return p_default;

	return std::wstring(f_buffer);
}

inline bool write_string(HKEY p_reg, const wchar_t *p_name, std::wstring p_value)
{
	return	RegSetValueEx(p_reg, p_name, 0, REG_SZ, reinterpret_cast<const BYTE *> (p_value.c_str()), (p_value.length() + 1) * sizeof (wchar_t)) == ERROR_SUCCESS;
}


#undef  SETTING_BOOLEAN
#define SETTING_BOOLEAN(p_name, p_default)	p_name = read_bool(g_registry, L#p_name, p_default);

#undef  SETTING_INTEGER
#define SETTING_INTEGER(p_name, p_default)	p_name = read_integer(g_registry, L#p_name, p_default);

#undef  SETTING_STRING
#define SETTING_STRING(p_name, p_default)	p_name = read_string(g_registry, L#p_name, p_default);

static HKEY		g_registry = nullptr;
static HANDLE	g_registry_changed = INVALID_HANDLE_VALUE;
static bool		g_registry_write = false;

void load()
{
	// open the key (if it's not already open)
	if (!g_registry)
	{
		if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\KinectWebCam", 0, KEY_READ, &g_registry) != ERROR_SUCCESS)
		{
			return;												// exit;
		}
	}

	// load the settings
	#include "settings_list.h"

	// start monitoring the key for changes
	if (g_registry_changed == INVALID_HANDLE_VALUE)
	{
		g_registry_changed = CreateEvent(nullptr, true, false, nullptr);
		RegNotifyChangeKeyValue(g_registry, FALSE, REG_NOTIFY_CHANGE_LAST_SET, g_registry_changed, TRUE);
	}
}

#undef  SETTING_BOOLEAN
#define SETTING_BOOLEAN(p_name, p_default)	write_bool(g_registry, L#p_name, p_name);

#undef  SETTING_INTEGER
#define SETTING_INTEGER(p_name, p_default)	write_integer(g_registry, L#p_name, p_name);

#undef  SETTING_STRING
#define SETTING_STRING(p_name, p_default)	write_string(g_registry, L#p_name, p_name);


void save()
{
	// close the registry-key if it's open read-only
	if (g_registry && !g_registry_write)
	{
		cleanup();
	}

	// open the key (if it's not already open)
	if (!g_registry)
	{
		if (RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\KinectWebCam", 0, NULL, 0, KEY_WRITE, NULL, &g_registry, NULL) != ERROR_SUCCESS)
		{
			return;												// exit;
		}

		g_registry_write = true;
	}

	// save the settings
	#include "settings_list.h"
}

void cleanup()
{
	if (g_registry)
	{
		RegCloseKey(g_registry);
		g_registry = nullptr;
	}
}

bool have_changed()
{
	bool f_result = (WaitForSingleObject(g_registry_changed, 0) == WAIT_OBJECT_0);

	if (f_result)
	{
		ResetEvent(g_registry_changed);
		RegNotifyChangeKeyValue(g_registry, FALSE, REG_NOTIFY_CHANGE_LAST_SET, g_registry_changed, TRUE);
	}

	return f_result;
}

}
