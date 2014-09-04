///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	Dll.cpp
//
// Purpose	: 	main entry-point
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory 
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "advapi32")
#pragma comment(lib, "winmm")
#pragma comment(lib, "ole32")
#pragma comment(lib, "oleaut32")
#pragma comment(lib, "Strmiids")

#define WIN32_LEAN_AND_MEAN
#include <streams.h>
#include <olectl.h>
#include <initguid.h>
#include <dllsetup.h>

#include "filter_video.h"
#include "com_utils.h"

STDAPI AMovieSetupRegisterServer( CLSID   clsServer, LPCWSTR szDescription, LPCWSTR szFileName, LPCWSTR szThreadingModel = L"Both", LPCWSTR szServerType = L"InprocServer32" );
STDAPI AMovieSetupUnregisterServer( CLSID clsServer );

// {2aac6912-8c07-4952-98d3-115a89c80c99}
DEFINE_GUID(CLSID_KinectWebCam,
0x2aac6912, 0x8c07, 0x4952, 0x98, 0xd3, 0x11, 0x5a, 0x89, 0xc8, 0x0c, 0x99);

const wchar_t FILTER_NAME_KINECT_WEBCAM[] = L"KinectWebCam";

const AMOVIESETUP_MEDIATYPE AMSMediaTypesKCam [] = 
{
	{ 
		&MEDIATYPE_Video, 
		&MEDIASUBTYPE_RGB32 
	},
	{ 
		&MEDIATYPE_Video, 
		&MEDIASUBTYPE_RGB24 
	}
};

const AMOVIESETUP_PIN AMSPinKCam =
{
    L"Output",													// Pin string name
    FALSE,														// Is it rendered
    TRUE,                										// Is it an output
    FALSE,														// Can we have none
    FALSE,														// Can we have many
    &CLSID_NULL,           										// Connects to filter
    NULL,                  										// Connects to pin
    sizeof(AMSMediaTypesKCam) / sizeof(AMOVIESETUP_MEDIATYPE),	// Number of pin media types
    AMSMediaTypesKCam     										// Pin Media types
};

const AMOVIESETUP_FILTER AMSFilterKCam =
{
    &CLSID_KinectWebCam,										// Filter CLSID
    FILTER_NAME_KINECT_WEBCAM,									// String name
    MERIT_DO_NOT_USE,											// Filter merit
	sizeof(AMSPinKCam) / sizeof(AMOVIESETUP_PIN),				// Number pins
    &AMSPinKCam             									// Pin details
};

CFactoryTemplate g_Templates[] = 
{
    {
        FILTER_NAME_KINECT_WEBCAM,
        &CLSID_KinectWebCam,
        CKCam::CreateInstance,
        NULL,
        &AMSFilterKCam
    },

};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI RegisterFilter(BOOL p_register, const GUID *p_clsid_category, const AMOVIESETUP_FILTER *p_filter, LPCWSTR p_filename)
{
	com_safe_ptr_t<IFilterMapper2>	f_mapper = nullptr;
    HRESULT f_result = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER, IID_IFilterMapper2, reinterpret_cast<void **>(&f_mapper));

	if (p_register)
    {
		if (SUCCEEDED(f_result))
		{
			f_result = AMovieSetupRegisterServer(*p_filter->clsID, p_filter->strName, p_filename, L"Both", L"InprocServer32");
		}

		if (SUCCEEDED(f_result))
		{
			IMoniker *pMoniker = nullptr;
			REGFILTER2 f_rf2;
			f_rf2.dwVersion	= 1;
			f_rf2.dwMerit	= MERIT_DO_NOT_USE;
			f_rf2.cPins		= p_filter->nPins;
			f_rf2.rgPins	= p_filter->lpPin;

			f_result = f_mapper->RegisterFilter(*p_filter->clsID, p_filter->strName, &pMoniker, p_clsid_category, NULL, &f_rf2);
		}
	}
	else
	{
		if (SUCCEEDED(f_result))
		{
			f_result = f_mapper->UnregisterFilter(p_clsid_category, 0, *p_filter->clsID);
        }

		if (SUCCEEDED(f_result))
		{
			f_result = AMovieSetupUnregisterServer (*p_filter->clsID);
		}
	}

	return f_result;
}

STDAPI RegisterFilters(BOOL bRegister)
{
    ASSERT(g_hInst != nullptr);

    HRESULT f_result = NOERROR;
    WCHAR   f_module_filename[MAX_PATH] = L"";

	if (GetModuleFileName(g_hInst, f_module_filename, MAX_PATH) == 0)
        return AmHresultFromWin32(GetLastError());
  
    f_result = CoInitialize(0);

	if (SUCCEEDED(f_result))
	{
		f_result = RegisterFilter(bRegister, &CLSID_VideoInputDeviceCategory, &AMSFilterKCam, f_module_filename);
	}

    CoFreeUnusedLibraries();
    CoUninitialize();
    return f_result;
}

STDAPI DllRegisterServer()
{
    return RegisterFilters(TRUE);
}

STDAPI DllUnregisterServer()
{
    return RegisterFilters(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}
