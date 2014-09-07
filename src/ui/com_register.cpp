///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	com_register.cpp
//
// Purpose	: 	Register a .dll as a com object
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#include "com_register.h"

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

typedef HRESULT (*DLLFUNC) (void);

bool execute_dll_function(const wchar_t *p_filename, const char *p_function)
{
	bool f_result = false;
	auto f_dll	  = LoadLibrary(p_filename);

	if (!f_dll)
	{
		return f_result;
	}

	DLLFUNC f_register_func = reinterpret_cast<DLLFUNC> (GetProcAddress(f_dll, p_function));

	if (f_register_func)
	{
		f_result = (f_register_func() == NO_ERROR);
	}

	FreeLibrary(f_dll);
	return f_result;
}


bool register_com_dll(const wchar_t *p_filename)
{
	return execute_dll_function(p_filename, "DllRegisterServer");
}

bool unregister_com_dll(const wchar_t *p_filename)
{
	return execute_dll_function(p_filename, "DllUnregisterServer");
}
