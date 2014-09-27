///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	filter_video.cpp
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

#define WIN32_LEAN_AND_MEAN
#include <streams.h>
#include <sstream>
#include <stdio.h>
#include <olectl.h>
#include <dvdmedia.h>

#include "filter_video.h"
#include "device.h"
#include "device_factory.h"
#include "settings.h"
#include "guid_filter.h"

inline int FrameIntervalFromRate(int framesPerSecond)
{
    return UNITS / framesPerSecond;
}

inline int FrameRateFromInterval(REFERENCE_TIME p_interval)
{
	return static_cast<int> (UNITS / p_interval);
}

inline DWORD CompressionFromPixelFormat(device::DevicePixelFormat p_pf)
{
	switch (p_pf)
	{
		case device::DPF_RGBA :	return BI_RGB;
		case device::DPF_RGB :	return BI_RGB;
		case device::DPF_YUY2 :	return MAKEFOURCC('Y','U','Y','2');
		default :				return BI_RGB;
	}
}

inline GUID MediaSubTypeFromPixelFormat(device::DevicePixelFormat p_pf)
{
	switch (p_pf)
	{
		case device::DPF_RGBA :	return MEDIASUBTYPE_RGB32;
		case device::DPF_RGB :	return MEDIASUBTYPE_RGB24;
		case device::DPF_YUY2 :	return MEDIASUBTYPE_YUY2;
		default :				return MEDIASUBTYPE_RGB32;
	}
}

inline device::DevicePixelFormat PixelFormatFromMediaSubType(GUID p_mst)
{
	if (p_mst == MEDIASUBTYPE_RGB32)
		return device::DPF_RGBA;
	else if (p_mst == MEDIASUBTYPE_RGB24)
		return device::DPF_RGB;
	else if (p_mst == MEDIASUBTYPE_YUY2)
		return device::DPF_YUY2;

	return device::DPF_RGBA;
}

inline device::Point2D smooth_focus_update(device::Point2D p_focus)
{
	static const int		MAX_SMOOTH_POINTS = 30;
	static device::Point2D	f_points[MAX_SMOOTH_POINTS] = {0};
	static int				f_index = 0;
	static int				f_count = 0;

	f_points[f_index] = p_focus;
	f_index			  = (f_index + 1) % MAX_SMOOTH_POINTS;
	f_count			  = min(f_count + 1, MAX_SMOOTH_POINTS);

	device::Point2D	f_result = f_points[0];

	for (int f_i = 1; f_i < MAX_SMOOTH_POINTS; ++f_i)
	{
		f_result.m_x += f_points[f_i].m_x;
		f_result.m_y += f_points[f_i].m_y;
	}

	return {f_result.m_x / f_count, f_result.m_y / f_count};
}

//////////////////////////////////////////////////////////////////////////
//  CKCam is the source filter which masquerades as a capture device
//////////////////////////////////////////////////////////////////////////

CUnknown * WINAPI CKCam::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    ASSERT(phr);
    CUnknown *punk = new CKCam(lpunk, phr);
    return punk;
}

CKCam::CKCam(LPUNKNOWN lpunk, HRESULT *phr) : 
    CSource(NAME("KinectWebCam"), lpunk, CLSID_KinectWebCam)
{
    ASSERT(phr);
    CAutoLock cAutoLock(&m_cStateLock);

    // create the one and only output pin
    m_paStreams = (CSourceStream **) new CKCamStream*[1];
    m_paStreams[0] = new CKCamStream(phr, this, L"KinectWebCam");
}

HRESULT CKCam::QueryInterface(REFIID riid, void **ppv)
{
    // forwards request for IAMStreamConfig, IKsPropertySet and IAMDroppedFrames to the pin
    if (riid == _uuidof(IAMStreamConfig)  || 
		riid == _uuidof(IAMDroppedFrames) ||
		riid == _uuidof(IKsPropertySet))
        return m_paStreams[0]->QueryInterface(riid, ppv);
    else
        return CSource::QueryInterface(riid, ppv);
}

//////////////////////////////////////////////////////////////////////////
// CKCamStream is the one and only output pin of CKCam which handles 
// all the stuff.
//////////////////////////////////////////////////////////////////////////

CKCamStream::CKCamStream(HRESULT *phr, CKCam *pParent, LPCWSTR pPinName) :
    CSourceStream(NAME("KinectWebCam"), phr, pParent, pPinName), 
	m_num_frames(0),
	m_num_dropped(0),
	m_pParent(pParent)
{
	// try to load the settings
	settings::load();

	// try to connect to a kinect V2
	if (!m_device && settings::KinectV2Enabled)
	{
		m_device = device::device_factory("kinect_v2");

		if (!m_device->connect_to_first())
			m_device = nullptr;
	}

	// fallback to an original kinect
	if (!m_device && settings::KinectV1Enabled)
	{
		m_device = device::device_factory("kinect");

		if (!m_device->connect_to_first())
			m_device = nullptr;
	}

	// store the default media type
	if (m_device)
	{
		GetMediaType(m_device->video_resolution_preferred() + 1, &m_mt);	// GetMediaType is 1 based
	}

	// initialize the camera focus in the center of the camera
	if (m_device)
	{
		auto f_native_res = m_device->video_resolution(m_device->video_resolution_native());
		m_focus.m_x = f_native_res.m_width / 2;
		m_focus.m_y = f_native_res.m_height / 2;
	}

	// disconnect from the device until playback is started
	if (m_device)
	{
		m_device->disconnect();
	}
}

CKCamStream::~CKCamStream()
{
	if (m_device)
	{
		m_device->disconnect();
	}
} 

HRESULT CKCamStream::QueryInterface(REFIID riid, void **ppv)
{   
    // Standard OLE stuff
    if(riid == _uuidof(IAMStreamConfig))
        *ppv = (IAMStreamConfig*) this;
    else if(riid == _uuidof(IKsPropertySet))
        *ppv = (IKsPropertySet*) this;
	else if(riid == _uuidof(IAMDroppedFrames)  )
		*ppv = (IAMDroppedFrames*) this;
	else
        return CSourceStream::QueryInterface(riid, ppv);

    AddRef();
    return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//  This is the routine where we create the data being output by the Virtual
//  Camera device.
//////////////////////////////////////////////////////////////////////////

HRESULT CKCamStream::FillBuffer(IMediaSample *pms)
{
	const REFERENCE_TIME AVG_FRAME_TIME = (reinterpret_cast<VIDEOINFOHEADER*> (m_mt.pbFormat))->AvgTimePerFrame;

	if (!m_device)
		return E_FAIL;

	// get the current time from the reference clock	
	IReferenceClock *f_clock = nullptr;
	m_pParent->GetSyncSource(&f_clock);
	
	if (f_clock)
	{
		f_clock->GetTime(&m_ref_time_current);
		f_clock->Release ();
	}

	// first frame : initialize values 
	if (m_num_frames <= 1)
	{
		m_ref_time_start = m_ref_time_current;
		m_time_dropped = 0;
	}

	REFERENCE_TIME f_now = m_time_stream;
	m_time_stream += AVG_FRAME_TIME;
	
	// compute generated stream time and compare to real elapsed time
	REFERENCE_TIME f_delta = ((m_ref_time_current - m_ref_time_start) - ((m_num_frames * AVG_FRAME_TIME) - AVG_FRAME_TIME));

	if (f_delta < m_time_dropped)
	{
		// it's too early - wait until it's time
		DWORD f_interval = static_cast<DWORD> (abs((f_delta - m_time_dropped) / 10000));

		if (f_interval >= 1)
		{
			Sleep(f_interval);
		}
	}
	else if (f_delta / AVG_FRAME_TIME > m_num_dropped)
	{	
		// newly dropped frame(s)
		m_num_dropped  = static_cast<long> (f_delta / AVG_FRAME_TIME);
		m_time_dropped = m_num_dropped * AVG_FRAME_TIME;

		// adjust the timestamps (find total real stream time from start time)
		f_now		  = m_ref_time_current - m_ref_time_start;
		m_time_stream = f_now + AVG_FRAME_TIME;

		pms->SetDiscontinuity(true);
	}

	pms->SetTime(&f_now, &m_time_stream);
	pms->SetSyncPoint(TRUE);

	if (settings::have_changed())
		settings::load();

	m_device->focus_set_joint(settings::TrackingJoint);

	// let the device update itself
	m_device->update();

	if (settings::TrackingEnabled && m_device->focus_availabe())
	{
		m_focus = smooth_focus_update(m_device->focus_point());
	}
	
	// copy the data to the output buffer
    BYTE *pData;
    pms->GetPointer(&pData);

	auto *f_pvi = reinterpret_cast<VIDEOINFOHEADER *> (m_mt.Format());
	m_device->color_data(m_focus.m_x, m_focus.m_y, f_pvi->bmiHeader.biWidth, f_pvi->bmiHeader.biHeight, f_pvi->bmiHeader.biBitCount, pData);

	++m_num_frames;
    return S_OK;
}

//
// Notify: Ignore quality management messages sent from the downstream filter
//

STDMETHODIMP CKCamStream::Notify(IBaseFilter * pSender, Quality q)
{
    return E_NOTIMPL;
} 

//////////////////////////////////////////////////////////////////////////
// This is called when the output format has been negotiated
//////////////////////////////////////////////////////////////////////////

HRESULT CKCamStream::SetMediaType(const CMediaType *pmt)
{
	DbgLog((LOG_TRACE, 1, "CKCamStream::SetMediaType : %x", pmt));
    DECLARE_PTR(VIDEOINFOHEADER, pvi, pmt->Format());

	DbgLog((LOG_TRACE, 1, "CKCamStream::SetMediaType : %d x %d x %d", pvi->bmiHeader.biWidth, pvi->bmiHeader.biHeight, pvi->bmiHeader.biBitCount));

	// make sure the device outputs in the correct format
	device::DeviceVideoResolution	f_devres;
	f_devres.m_width			= pvi->bmiHeader.biWidth;
	f_devres.m_height			= pvi->bmiHeader.biHeight;
	f_devres.m_bits_per_pixel	= pvi->bmiHeader.biBitCount;
	f_devres.m_framerate		= FrameRateFromInterval(pvi->AvgTimePerFrame);
	f_devres.m_pixel_format		= PixelFormatFromMediaSubType(*pmt->Subtype());
	m_device->video_set_resolution(f_devres);

	// see documentation of BITMAPINFOHEADER (http://msdn.microsoft.com/en-us/library/windows/desktop/dd318229%28v=vs.85%29.aspx) for more details
	// - For uncompressed RGB bitmaps, if biHeight is positive, the bitmap is a bottom-up DIB with the origin at the lower left corner. 
	//	 If biHeight is negative, the bitmap is a top-down DIB with the origin at the upper left corner.
	// - For YUV bitmaps, the bitmap is always top-down, regardless of the sign of biHeight. 
	if (pvi->bmiHeader.biCompression == BI_RGB || pvi->bmiHeader.biCompression)
		m_device->video_flip_output(pvi->bmiHeader.biHeight > 0);
	else
		m_device->video_flip_output(false);

	return CSourceStream::SetMediaType(pmt);
}

// See Directshow help topic for IAMStreamConfig for details on this method
HRESULT CKCamStream::GetMediaType(int iPosition, CMediaType *pmt)
{
	DbgLog((LOG_TRACE, 1, "GetMediaType (iPosition = %d)", iPosition));

	if (!m_device)
		return E_FAIL;

    if (iPosition < 0) return E_INVALIDARG;
    if (iPosition > m_device->video_resolution_count()) return VFW_S_NO_MORE_ITEMS;

	if (iPosition == 0)
	{
		// return the default (preferred) resolution
		*pmt = m_mt;
		return S_OK;																// exit !!!
	}

	// check the device for information of this resolution
	auto f_devres = m_device->video_resolution(iPosition - 1);

	// fill in the VIDEOINFO_HEADER
    DECLARE_PTR(VIDEOINFOHEADER, pvi, pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER)));
    ZeroMemory(pvi, sizeof(VIDEOINFOHEADER));

    pvi->bmiHeader.biCompression	= CompressionFromPixelFormat(f_devres.m_pixel_format);
    pvi->bmiHeader.biBitCount		= f_devres.m_bits_per_pixel;
    pvi->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth			= f_devres.m_width;
    pvi->bmiHeader.biHeight			= f_devres.m_height;
    pvi->bmiHeader.biPlanes			= 1;
    pvi->bmiHeader.biSizeImage		= GetBitmapSize(&pvi->bmiHeader);
    pvi->bmiHeader.biClrImportant	= 0;

    pvi->AvgTimePerFrame			= FrameIntervalFromRate(f_devres.m_framerate);

    SetRectEmpty(&(pvi->rcSource));		// we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget));		// no particular destination rectangle

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE);

    // Work out the GUID for the subtype from the header info.
    const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
    pmt->SetSubtype(&SubTypeGUID);
    pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);

	DbgLog((LOG_TRACE, 1, "GetMediaType (iPosition = %d) <= %d x %d x %d", iPosition, pvi->bmiHeader.biWidth, pvi->bmiHeader.biHeight, pvi->bmiHeader.biBitCount));
    
    return S_OK;
}

HRESULT CKCamStream::CheckMediaType(const CMediaType *pMediaType)
{
	// The CheckMediaType method determines if the pin accepts a specific media type. 
	// MSDN says to "Only accept the preferred media type (See SetFormat for more information)"
	//	but there applications that call CheckMediaType with a different media type than they call SetFormat (e.g. Flash in Chrome)
	//	=> we just check some crucial parameters of the requested media type match with something we offer
	DbgLog((LOG_TRACE, 1, "CheckMediaType"));

	if (!m_device)
		return E_FAIL;

    DECLARE_PTR(VIDEOINFOHEADER, f_pvi, pMediaType->pbFormat);
	DbgLog((LOG_TRACE, 1, "... CheckMediaType (%dx%dx%d)", f_pvi->bmiHeader.biWidth, f_pvi->bmiHeader.biHeight, f_pvi->bmiHeader.biBitCount));

	CAutoLock f_lock(m_pFilter->pStateLock());	// XXX not needed anymore ?
	bool f_ok = false;

	for (int f_idx = 0; !f_ok && f_idx < m_device->video_resolution_count(); ++f_idx)
	{
		auto f_res = m_device->video_resolution(f_idx);

		f_ok = (f_res.m_width == f_pvi->bmiHeader.biWidth &&
			    f_res.m_height == abs(f_pvi->bmiHeader.biHeight) &&
			    f_res.m_bits_per_pixel == f_pvi->bmiHeader.biBitCount &&
				CompressionFromPixelFormat(f_res.m_pixel_format) == f_pvi->bmiHeader.biCompression);
	}

	DbgLog((LOG_TRACE, 1, "... CheckMediaType (%s)", (f_ok) ? "OK" : "NOK"));
	return (f_ok) ? S_OK : E_INVALIDARG;
}

// This method is called after the pins are connected to allocate buffers to stream data
HRESULT CKCamStream::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pProperties)
{
	DbgLog((LOG_TRACE, 1, "DecideBufferSize"));

    CAutoLock cAutoLock(m_pFilter->pStateLock());

	auto *f_pvi = reinterpret_cast<VIDEOINFOHEADER *> (m_mt.Format());
    pProperties->cBuffers = 1;
    pProperties->cbBuffer = f_pvi->bmiHeader.biSizeImage;

	// specify the buffer requirements
	ALLOCATOR_PROPERTIES f_actual;
    HRESULT f_result = pAlloc->SetProperties(pProperties, &f_actual);

    if (FAILED (f_result))
		return f_result;

	// sufficient memory ?
    if (f_actual.cbBuffer < pProperties->cbBuffer) 
		return E_FAIL;

    return NOERROR;
} 

HRESULT CKCamStream::OnThreadCreate()
{
    m_time_stream  = 0;
	m_num_dropped = 0;
	m_num_frames  = 0;

	// be sure to refresh the settings
	settings::load();

	// reconnect to the device
	if (m_device)
	{
		m_device->connect_to_first();
	}

    return NOERROR;
}

HRESULT CKCamStream::OnThreadDestroy()
{
	// disconnect from the device
	if (m_device)
	{
		m_device->disconnect();
	}

	// stop monitoring settings
	settings::cleanup();

    return NOERROR;
}

//////////////////////////////////////////////////////////////////////////
//  IAMStreamConfig
//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CKCamStream::SetFormat(AM_MEDIA_TYPE *pmt)
{
	// from MSDN (http://msdn.microsoft.com/en-us/library/windows/desktop/dd319788%28v=vs.85%29.aspx)
	// The SetFormat method sets the output format on the pin.
	// If the output pin is not connected, and the pin supports the specified media type, return S_OK. 
	//	Store the media type and offer it as format number zero in the CBasePin::GetMediaType method. 
	//	Do not offer other formats, and reject other formats in the CBasePin::CheckMediaType method.
	// If the pin is already connected, and the pin supports the media type, reconnect the pin with that type. 
	//	If the other pin rejects the new type, return VFW_E_INVALIDMEDIATYPE and restore the original connection.
	DbgLog((LOG_TRACE, 1, "SetFormat : %x", pmt));

	if (!m_device)
		return E_FAIL;

	if (!pmt)
	{
		// from MSDN: With some filters, you can call this method with the value NULL to reset the pin to its default format.
		GetMediaType(m_device->video_resolution_preferred(), &m_mt);
		return S_OK;
	}

	// check if the video format is supported by the device (might be redundant)
	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)(pmt->pbFormat);

	DbgLog((LOG_TRACE, 1, "SetFormat : %d x %d x %d", pvi->bmiHeader.biWidth, pvi->bmiHeader.biHeight, pvi->bmiHeader.biBitCount));
	
	bool f_ok = false;

	for (int f_idx = 0; !f_ok && f_idx < m_device->video_resolution_count(); ++f_idx)
	{
		auto f_res = m_device->video_resolution(f_idx);
		
		f_ok = (f_res.m_width == pvi->bmiHeader.biWidth &&
			    f_res.m_height == pvi->bmiHeader.biHeight &&
			    f_res.m_bits_per_pixel == pvi->bmiHeader.biBitCount);
	}

	m_mt = *pmt;

	DbgLog((LOG_TRACE, 1, "SetFormat : %s", (f_ok) ? "OK" : "NOK"));

	return (f_ok) ? S_OK : VFW_E_INVALIDMEDIATYPE;
}

HRESULT STDMETHODCALLTYPE CKCamStream::GetFormat(AM_MEDIA_TYPE **ppmt)
{
    *ppmt = CreateMediaType(&m_mt);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CKCamStream::GetNumberOfCapabilities(int *piCount, int *piSize)
{
	if (!m_device)
		return E_FAIL;

    *piCount = m_device->video_resolution_count();
    *piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CKCamStream::GetStreamCaps(int iIndex, AM_MEDIA_TYPE **pmt, BYTE *pSCC)
{
	if (!m_device)
		return E_FAIL;

    *pmt = CreateMediaType(&m_mt);
    DECLARE_PTR(VIDEOINFOHEADER, pvi, (*pmt)->pbFormat);

	DbgLog((LOG_TRACE, 1, "GetStreamCaps (iPosition = %d)", iIndex));

	auto f_devres = m_device->video_resolution(iIndex);

    pvi->bmiHeader.biCompression = CompressionFromPixelFormat(f_devres.m_pixel_format);
    pvi->bmiHeader.biBitCount    = f_devres.m_bits_per_pixel;
    pvi->bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth       = f_devres.m_width;
    pvi->bmiHeader.biHeight      = f_devres.m_height;
    pvi->bmiHeader.biPlanes      = 1;
    pvi->bmiHeader.biSizeImage   = GetBitmapSize(&pvi->bmiHeader);
    pvi->bmiHeader.biClrImportant = 0;

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    (*pmt)->majortype				= MEDIATYPE_Video;
    (*pmt)->subtype					= MediaSubTypeFromPixelFormat(f_devres.m_pixel_format);
    (*pmt)->formattype				= FORMAT_VideoInfo;
    (*pmt)->bTemporalCompression	= FALSE;
    (*pmt)->bFixedSizeSamples		= FALSE;
    (*pmt)->lSampleSize				= pvi->bmiHeader.biSizeImage;
    (*pmt)->cbFormat				= sizeof(VIDEOINFOHEADER);
    
    DECLARE_PTR(VIDEO_STREAM_CONFIG_CAPS, pvscc, pSCC);
    
    pvscc->guid					= FORMAT_VideoInfo;
    pvscc->VideoStandard		= AnalogVideo_None;
    pvscc->InputSize.cx			= f_devres.m_width;
    pvscc->InputSize.cy			= f_devres.m_height;
    pvscc->MinCroppingSize.cx	= f_devres.m_width;
    pvscc->MinCroppingSize.cy	= f_devres.m_height;
    pvscc->MaxCroppingSize.cx	= f_devres.m_width;
    pvscc->MaxCroppingSize.cy	= f_devres.m_height;
    pvscc->CropGranularityX		= 0;
    pvscc->CropGranularityY		= 0;
    pvscc->CropAlignX			= 0;
    pvscc->CropAlignY			= 0;

    pvscc->MinOutputSize.cx		= f_devres.m_width;
    pvscc->MinOutputSize.cy		= f_devres.m_height;
    pvscc->MaxOutputSize.cx		= f_devres.m_width;
    pvscc->MaxOutputSize.cy		= f_devres.m_height;
    pvscc->OutputGranularityX	= 0;
    pvscc->OutputGranularityY	= 0;
    pvscc->StretchTapsX			= 0;
    pvscc->StretchTapsY			= 0;
    pvscc->ShrinkTapsX			= 0;
    pvscc->ShrinkTapsY			= 0;

	int bitsPerFrame = f_devres.m_width * f_devres.m_height * f_devres.m_bits_per_pixel;
    pvscc->MinFrameInterval = FrameIntervalFromRate(f_devres.m_framerate);
    pvscc->MaxFrameInterval = FrameIntervalFromRate(10);
    pvscc->MinBitsPerSecond = bitsPerFrame * 10;
    pvscc->MaxBitsPerSecond = bitsPerFrame * f_devres.m_framerate;

    return S_OK;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
//			IAMDroppedFrames
///////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CKCamStream::GetNumNotDropped (long* plNotDropped)
{
	if (!plNotDropped) 
		return E_POINTER;

	*plNotDropped = m_num_frames;
	return NOERROR;
}

HRESULT STDMETHODCALLTYPE CKCamStream::GetNumDropped (long* plDropped)
{
	if (!plDropped) 
		return E_POINTER;
	
	*plDropped = m_num_dropped;
	return NOERROR;
}

HRESULT STDMETHODCALLTYPE CKCamStream::GetDroppedInfo (long lSize, long *plArraym, long* plNumCopied)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CKCamStream::GetAverageFrameSize (long* plAverageSize)
{
	if(!plAverageSize)
		return E_POINTER;

	auto *f_pvi		= reinterpret_cast<VIDEOINFOHEADER *> (m_mt.Format());
	*plAverageSize  = f_pvi->bmiHeader.biSizeImage;
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
// IKsPropertySet
//////////////////////////////////////////////////////////////////////////

HRESULT CKCamStream::Set(REFGUID guidPropSet, DWORD dwID, void *pInstanceData, 
                        DWORD cbInstanceData, void *pPropData, DWORD cbPropData)
{
    return E_NOTIMPL;
}

// Get: Return the pin category (our only property). 
HRESULT CKCamStream::Get(
    REFGUID guidPropSet,   // Which property set.
    DWORD dwPropID,        // Which property in that set.
    void *pInstanceData,   // Instance data (ignore).
    DWORD cbInstanceData,  // Size of the instance data (ignore).
    void *pPropData,       // Buffer to receive the property data.
    DWORD cbPropData,      // Size of the buffer.
    DWORD *pcbReturned     // Return the size of the property.
)
{
    if (guidPropSet != AMPROPSETID_Pin)             return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY)        return E_PROP_ID_UNSUPPORTED;
    if (pPropData == NULL && pcbReturned == NULL)   return E_POINTER;
    
    if (pcbReturned) *pcbReturned = sizeof(GUID);
    if (pPropData == NULL)          return S_OK;			// caller just wants to know the size. 
    if (cbPropData < sizeof(GUID))  return E_UNEXPECTED;	// The buffer is too small.
        
    *(GUID *) pPropData = PIN_CATEGORY_CAPTURE;
    return S_OK;
}

// QuerySupported: Query whether the pin supports the specified property.
HRESULT CKCamStream::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
    if (guidPropSet != AMPROPSETID_Pin)		 return E_PROP_SET_UNSUPPORTED;
    if (dwPropID != AMPROPERTY_PIN_CATEGORY) return E_PROP_ID_UNSUPPORTED;

	// We support getting this property, but not setting it.
    if (pTypeSupport) *pTypeSupport = KSPROPERTY_SUPPORT_GET; 
    return S_OK;
}
