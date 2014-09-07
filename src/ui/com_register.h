///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	com_register.h
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

#ifndef KW_COM_REGISTER_H
#define KW_COM_REGISTER_H

bool register_com_dll(const wchar_t *p_filename);
bool unregister_com_dll(const wchar_t *p_filename);

#endif // KW_COM_REGISTER_H