///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	ds_capture.cpp
//
// Purpose	: 	Preview a DirectShow capture filter
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#include "ds_capture.h"
#include "com_utils.h"

DSVideoCapture::DSVideoCapture() :	m_graph(nullptr),
                                    m_builder(nullptr),
                                    m_control(nullptr),
                                    m_video_window(nullptr),
                                    m_source(nullptr),
									m_config(nullptr)
{
}

bool DSVideoCapture::initialize(GUID p_capture_guid)
{
    if (!create_capture_filter(p_capture_guid))
		return false;

	return true;
}

void DSVideoCapture::preview_device(RECT p_window, HWND p_video_parent)
{
	// graph
    if (!graph_initialize())
		return;

	// video window
	if (!video_window_initialize(p_window, p_video_parent))
		return;

	// start the preview
	m_control->Run();
}

void DSVideoCapture::preview_shutdown()
{
	if (m_control)
		m_control->Stop();

	graph_shutdown();
}

bool DSVideoCapture::graph_initialize()
{
    HRESULT f_result = S_OK;

    // create a FilterGraph
    if (SUCCEEDED(f_result))
    {
        f_result = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER, IID_IFilterGraph2, reinterpret_cast<void **> (&m_graph));
    }

    // create a CaptureGraphBuilder
    if (SUCCEEDED(f_result))
    {
       f_result = CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, reinterpret_cast<void **> (&m_builder));
    }

    if (SUCCEEDED(f_result))
    {
		m_builder->SetFiltergraph(m_graph);
	}	

    // get interface to be used later
    if (SUCCEEDED(f_result))
    {
        f_result = m_graph->QueryInterface(IID_IMediaControl, reinterpret_cast<void**> (&m_control));
    }

    if (SUCCEEDED(f_result))
    {
        f_result = m_graph->QueryInterface(IID_IVideoWindow, reinterpret_cast<void **> (&m_video_window));
    }

	// add the capture device to the graph
	if (SUCCEEDED (f_result))
	{
		m_graph->AddFilter(m_source, L"Video Capture");
	}

	// create the video renderer
	if (SUCCEEDED (f_result))
	{
		f_result = CoCreateInstance(CLSID_VideoRenderer, nullptr, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void **> (&m_renderer));
	}

	if (SUCCEEDED (f_result))
	{
		f_result = m_graph->AddFilter(m_renderer, L"Video Renderer");
	}

	// render the preview pin on the video capture filter
	if (SUCCEEDED (f_result))
	{
		f_result = m_builder->RenderStream (&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, m_source, nullptr, m_renderer);
	}

#ifdef _DEBUG
	if (SUCCEEDED(f_result))
	{
		rot_add_graph();
	}
#endif

    return SUCCEEDED (f_result);
}

bool DSVideoCapture::graph_shutdown()
{
    // close the video window
	if (m_video_window)
    {
        m_video_window->put_Owner(NULL);
        m_video_window->put_Visible(OAFALSE);
    }

	// remove the graph from the running object table
#ifdef _DEBUG
	rot_remove_graph();
#endif
		
    // release the interface
	com_safe_release(&m_renderer);
	com_safe_release(&m_source);
    com_safe_release(&m_control);
	com_safe_release(&m_config);
    com_safe_release(&m_video_window);
    com_safe_release(&m_graph);
    com_safe_release(&m_builder);

    return true;
}

bool DSVideoCapture::create_capture_filter(GUID p_guid)
{
	// create the source filter
    HRESULT f_result = CoCreateInstance(p_guid, nullptr, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void **>(&m_source));

	// get an interface to configure the stream
	if (SUCCEEDED (f_result))
	{
		f_result = m_source->QueryInterface(IID_IAMStreamConfig, reinterpret_cast<void **>(&m_config));
	}

	// retrieve the list of supported resolutions
	int f_caps_count;
	int f_caps_size;
	
	if (SUCCEEDED (f_result))
	{
		m_config->GetNumberOfCapabilities(&f_caps_count, &f_caps_size);
	}
	
	for (int f_idx=0; SUCCEEDED(f_result) && f_idx < f_caps_count; ++f_idx)
	{
		VIDEO_STREAM_CONFIG_CAPS f_scc;
		AM_MEDIA_TYPE *			 f_mt;
	
		f_result = m_config->GetStreamCaps(f_idx, &f_mt, (BYTE*) &f_scc);
		VIDEOINFOHEADER *f_vi = reinterpret_cast<VIDEOINFOHEADER *> (f_mt->pbFormat);

		m_resolutions.push_back({f_idx, f_vi->bmiHeader.biWidth, f_vi->bmiHeader.biHeight, f_vi->bmiHeader.biBitCount});
	}

	return SUCCEEDED(f_result);
}

bool DSVideoCapture::video_window_initialize(RECT p_window, HWND p_video_parent)
{
	HRESULT f_result = S_OK;

    // set the video window to be a child of specified parent window
	if (SUCCEEDED(f_result) && p_video_parent)
	{
		f_result = m_video_window->put_Owner(reinterpret_cast<OAHWND>(p_video_parent));
	}
    
    // window style
	if (SUCCEEDED(f_result) && p_video_parent)
	{
		f_result = m_video_window->put_WindowStyle(WS_CHILD | WS_CLIPCHILDREN);
	}

	// size
	if (SUCCEEDED(f_result) && p_video_parent)
	{
		m_preview_parent = p_video_parent;
		video_window_resize(p_window);
	}

	// show the video window
	if (SUCCEEDED(f_result))
	{
		f_result = m_video_window->put_Visible(OATRUE);
	}

	return SUCCEEDED(f_result);
}

void DSVideoCapture::video_window_resize(RECT p_dimension)
{
	if (!m_video_window || !m_preview_parent)
		return;

    m_video_window->SetWindowPosition(p_dimension.left, p_dimension.top, p_dimension.right, p_dimension.bottom);
}

void DSVideoCapture::video_change_resolution(int p_resolution)
{
	HRESULT f_result = S_OK;

	// stop the preview
	if (SUCCEEDED(f_result))
	{
		f_result = m_control->Stop();
		m_video_window->put_Visible(OAFALSE);
	}

	// disconnect the source and remove the filters between the source and the renderer
	if (SUCCEEDED(f_result))
	{
		graph_remove_downstream(m_source, m_renderer);
	}

	// change the resolution
	VIDEO_STREAM_CONFIG_CAPS f_scc;
    AM_MEDIA_TYPE *			 f_mt;

    f_result = m_config->GetStreamCaps(p_resolution, &f_mt, (BYTE*) &f_scc);

    if (SUCCEEDED(f_result))
    {
		f_result = m_config->SetFormat(f_mt);
	}

	// reconnect the source with the renderer
    if (SUCCEEDED(f_result))
    {
		f_result = m_builder->RenderStream (&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, m_source, nullptr, m_renderer);
	}

	// restart the preview
    if (SUCCEEDED(f_result))
    {
		m_video_window->put_Visible(OATRUE);
		m_control->Run();
	}
}

void DSVideoCapture::graph_remove_downstream(IBaseFilter *p_start, IBaseFilter *p_end)
{
    com_safe_ptr_t<IEnumPins> f_pins = nullptr;
    PIN_INFO f_pininfo;

    if (!p_start)
        return;

    HRESULT f_result = p_start->EnumPins(&f_pins);
    f_pins->Reset();

    while (f_result == NOERROR)
    {
		com_safe_ptr_t<IPin>	f_pin;
        f_result = f_pins->Next(1, &f_pin, nullptr);

        if (f_result == S_OK && f_pin.get())
        {
			com_safe_ptr_t<IPin>	f_pin_to;
            f_pin->ConnectedTo(&f_pin_to);

            if (f_pin_to.get())
            {
                f_result = f_pin_to->QueryPinInfo(&f_pininfo);
				com_safe_ptr_t<IBaseFilter> f_filter = f_pininfo.pFilter;

                if (f_result == NOERROR && f_pininfo.dir == PINDIR_INPUT && f_pininfo.pFilter != p_end)
                {
					graph_remove_downstream(f_pininfo.pFilter, p_end);

					m_graph->Disconnect(f_pin.get());
					m_graph->Disconnect(f_pin_to.get());
					m_graph->RemoveFilter(f_pininfo.pFilter);
                }
            }
        }
    }
}

#ifdef _DEBUG

void DSVideoCapture::rot_add_graph()
{
    IMoniker * pMoniker;
    IRunningObjectTable *pROT;
    WCHAR wsz[128];
    HRESULT hr;
	
	if (m_graph == nullptr)
		return;

    if (FAILED(GetRunningObjectTable(0, &pROT)))
        return;

    hr = StringCchPrintfW(wsz, NUMELMS(wsz), L"FilterGraph %08x pid %08x\0", (DWORD_PTR) m_graph, GetCurrentProcessId());

    hr = CreateItemMoniker(L"!", wsz, &pMoniker);
    if (SUCCEEDED(hr)) 
    {
        // Use the ROTFLAGS_REGISTRATIONKEEPSALIVE to ensure a strong reference
        // to the object.  Using this flag will cause the object to remain
        // registered until it is explicitly revoked with the Revoke() method.
        //
        // Not using this flag means that if GraphEdit remotely connects
        // to this graph and then GraphEdit exits, this object registration 
        // will be deleted, causing future attempts by GraphEdit to fail until
        // this application is restarted or until the graph is registered again.
        hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, m_graph, pMoniker, &m_rot_id);
        pMoniker->Release();
    }

    pROT->Release();
}

void DSVideoCapture::rot_remove_graph()
{
    IRunningObjectTable *pROT;

    if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
    {
        pROT->Revoke(m_rot_id);
        pROT->Release();
    }
}

#endif