///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	filter_video.h
//
// Purpose	: 	video filter for the kinect webcam
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory 
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#ifndef KW_FILTER_VIDEO_H
#define KW_FILTER_VIDEO_H

#define DECLARE_PTR(type, ptr, expr) type* ptr = (type*)(expr);

EXTERN_C const GUID CLSID_KinectWebCam;

#include "device.h"
#include <memory>

class CKCam : public CSource
{
	public:
		//////////////////////////////////////////////////////////////////////////
		//  IUnknown
		//////////////////////////////////////////////////////////////////////////
		static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
		STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
		IFilterGraph *GetGraph() {return m_pGraph;}

	private:
	   CKCam (LPUNKNOWN lpunk, HRESULT *phr);
};

class CKCamStream : public CSourceStream,  public IAMDroppedFrames, public IAMStreamConfig, public IKsPropertySet
{
	// interfaces
	public :
		//////////////////////////////////////////////////////////////////////////
		//  IUnknown
		//////////////////////////////////////////////////////////////////////////
		STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
		STDMETHODIMP_(ULONG) AddRef() { return GetOwner()->AddRef(); }                                                          \
		STDMETHODIMP_(ULONG) Release() { return GetOwner()->Release(); }

		//////////////////////////////////////////////////////////////////////////
		//  IQualityControl
		//////////////////////////////////////////////////////////////////////////
		STDMETHODIMP Notify(IBaseFilter * pSender, Quality q);

		//////////////////////////////////////////////////////////////////////////
		//  IAMStreamConfig
		//////////////////////////////////////////////////////////////////////////
		HRESULT STDMETHODCALLTYPE SetFormat(AM_MEDIA_TYPE *pmt);
		HRESULT STDMETHODCALLTYPE GetFormat(AM_MEDIA_TYPE **ppmt);
		HRESULT STDMETHODCALLTYPE GetNumberOfCapabilities(int *piCount, int *piSize);
		HRESULT STDMETHODCALLTYPE GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC);

		//////////////////////////////////////////////////////////////////////////
		//  IAMDroppedFrames
		//////////////////////////////////////////////////////////////////////////
		HRESULT STDMETHODCALLTYPE GetAverageFrameSize( long* plAverageSize);
		HRESULT STDMETHODCALLTYPE GetDroppedInfo(long  lSize,long* plArray,long* plNumCopied);
		HRESULT STDMETHODCALLTYPE GetNumDropped(long *plDropped);
		HRESULT STDMETHODCALLTYPE GetNumNotDropped(long *plNotDropped);

		//////////////////////////////////////////////////////////////////////////
		//  IKsPropertySet
		//////////////////////////////////////////////////////////////////////////
		HRESULT STDMETHODCALLTYPE Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, DWORD cbInstanceData, void *pPropData, DWORD cbPropData);
		HRESULT STDMETHODCALLTYPE Get(REFGUID guidPropSet, DWORD dwPropID, void *pInstanceData,DWORD cbInstanceData, void *pPropData, DWORD cbPropData, DWORD *pcbReturned);
		HRESULT STDMETHODCALLTYPE QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport);
		
		//////////////////////////////////////////////////////////////////////////
		//  CSourceStream
		//////////////////////////////////////////////////////////////////////////
		CKCamStream(HRESULT *phr, CKCam *pParent, LPCWSTR pPinName);
		~CKCamStream();

		HRESULT FillBuffer(IMediaSample *pms);
		HRESULT DecideBufferSize(IMemAllocator *pIMemAlloc, ALLOCATOR_PROPERTIES *pProperties);
		HRESULT CheckMediaType(const CMediaType *pMediaType);
		HRESULT GetMediaType(int iPosition, CMediaType *pmt);
		HRESULT SetMediaType(const CMediaType *pmt);
		HRESULT OnThreadCreate(void);
		HRESULT OnThreadDestroy(void);
    
	// variables
	private:
		CKCam *	m_pParent;

		// the device 
		std::unique_ptr<device::Device>	m_device;

		// timing (dropped frames)
		long			m_num_frames;
		long			m_num_dropped;
		REFERENCE_TIME	m_ref_time_current;		// Graphmanager clock time (real time)
		REFERENCE_TIME 	m_ref_time_start;		// Graphmanager time at the start of the stream (real time)
		REFERENCE_TIME	m_time_stream;			// running timestamp (stream time - using normal average time per frame)
		REFERENCE_TIME 	m_time_dropped;			// total time in dropped frames
};

#endif // KW_FILTER_VIDEO_H