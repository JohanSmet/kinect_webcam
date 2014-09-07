///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	ds_capture.h
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

#ifndef KW_DS_CAPTURE_H
#define KW_DS_CAPTURE_H

#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <dshow.h>
#include <vector>

struct DSVideoResolution
{
	int		m_id;
	int		m_width;
	int		m_height;
	int		m_bpp;
};

typedef	std::vector<DSVideoResolution>	VideoResolutions;

class DSVideoCapture
{
    // member functions
    public :
		// construction
        DSVideoCapture();

		// initialize : create the capture filter without starting the previeuw
		bool initialize(GUID p_capture_guid);

		// create a preview of the specified device in the specified window
        void preview_device(RECT p_dimension, HWND p_video_parent = nullptr);
		void preview_shutdown();
		
		// control the position and size of the preview window
		void video_window_resize(RECT p_dimesion);
	
		// resolution
		void video_change_resolution(int p_resolution);
		inline const VideoResolutions &video_resolutions() const {return m_resolutions;}

    // helper functions
    private :
        bool graph_initialize();
        bool graph_shutdown();
		void graph_remove_downstream(IBaseFilter *p_start, IBaseFilter *p_end);

        bool create_capture_filter(GUID p_guid);

		bool video_window_initialize(RECT p_dimension, HWND p_video_parent);

		#ifdef _DEBUG
			void rot_add_graph();
			void rot_remove_graph();
		#endif

    // member variables
    private :
        IFilterGraph2 *			m_graph;
        ICaptureGraphBuilder2 * m_builder;
        IMediaControl *			m_control;
        IVideoWindow *			m_video_window;
        IBaseFilter *			m_source;
		IBaseFilter *			m_renderer;
		IAMStreamConfig	*		m_config;

		HWND					m_preview_parent;

		VideoResolutions		m_resolutions;

		#ifdef _DEBUG
			DWORD				m_rot_id;
		#endif
};


#endif // KW_DS_CAPTURE_H
