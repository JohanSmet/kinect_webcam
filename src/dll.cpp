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

STDAPI RegisterFilters( BOOL bRegister )
{
    HRESULT hr = NOERROR;
    WCHAR achFileName[MAX_PATH];
    char achTemp[MAX_PATH];
    ASSERT(g_hInst != nullptr);

    if( 0 == GetModuleFileNameA(g_hInst, achTemp, sizeof(achTemp))) 
        return AmHresultFromWin32(GetLastError());

    MultiByteToWideChar(CP_ACP, 0L, achTemp, lstrlenA(achTemp) + 1, 
                       achFileName, NUMELMS(achFileName));
  
    hr = CoInitialize(0);

    if (bRegister)
    {
        hr = AMovieSetupRegisterServer(CLSID_KinectWebCam, FILTER_NAME_KINECT_WEBCAM, achFileName, L"Both", L"InprocServer32");
    }

    if (SUCCEEDED(hr))
    {
		com_safe_ptr_t<IFilterMapper2>	fm = nullptr;

		hr = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER, IID_IFilterMapper2, reinterpret_cast<void **>(&fm));

        if (SUCCEEDED(hr) && bRegister)
        {
			IMoniker *pMoniker = nullptr;
			REGFILTER2 rf2;
			rf2.dwVersion	= 1;
			rf2.dwMerit		= MERIT_DO_NOT_USE;
			rf2.cPins		= sizeof(AMSPinKCam) / sizeof(AMOVIESETUP_PIN);
			rf2.rgPins		= &AMSPinKCam;

			hr = fm->RegisterFilter(CLSID_KinectWebCam, FILTER_NAME_KINECT_WEBCAM, &pMoniker, &CLSID_VideoInputDeviceCategory, NULL, &rf2);
        }

        if (SUCCEEDED(hr) && !bRegister)
        {
			hr = fm->UnregisterFilter(&CLSID_VideoInputDeviceCategory, 0, CLSID_KinectWebCam);
        }
    }

    if( SUCCEEDED(hr) && !bRegister)
        hr = AMovieSetupUnregisterServer( CLSID_KinectWebCam );

    CoFreeUnusedLibraries();
    CoUninitialize();
    return hr;
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
