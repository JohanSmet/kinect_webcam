///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	com_utils.h
//
// Purpose	: 	some things to make working with COM a bit cleaner
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#ifndef KS_COM_UTILS_H
#define KS_COM_UTILS_H

template <class T>
inline void com_safe_release(T **p_interface)
{
    if (*p_interface)
    {
        (*p_interface)->Release();
        *p_interface = nullptr;
    }
}

template <class T>
class com_safe_ptr_t
{
	public :
		com_safe_ptr_t(T *p_ptr = nullptr) : m_ptr(p_ptr) {}
		~com_safe_ptr_t() { com_safe_release(&m_ptr); }

		inline T **operator &() {return &m_ptr;}
		inline T * operator->() {return m_ptr;}

		inline T *get() {return m_ptr;}

	private :
		T *m_ptr;
};

#endif // KS_COM_UTILS_H
