///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	mainwindow.cpp
//
// Purpose	: 	Main application window
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "ds_capture.h"
#include "qt_utils.h"
#include "settings.h"
#include "com_register.h"
#include "guid_filter.h"

MainWindow::MainWindow(QWidget *p_parent) :	QMainWindow(p_parent),
											ui(new Ui::MainWindow),
											m_preview(std::make_unique<DSVideoCapture>()),
											m_device_ok(false),
											m_updating_ui(true)
{
    ui->setupUi(this);

	// settings
	settings::load();
	ui_from_settings();

	// initialize capture device
	init_device();

	m_updating_ui = false;
}

MainWindow::~MainWindow()
{
	if (m_preview)
		m_preview->shutdown();

    delete ui;
}

///////////////////////////////////////////////////////////////////////////////
//
// events
//

void MainWindow::resizeEvent(QResizeEvent *p_event)
{
	if (m_preview)
	{
		m_preview->video_window_resize({ui->widget->x(), ui->widget->y(), ui->widget->width(), ui->widget->height()});
	}

	QMainWindow::resizeEvent(p_event);
}

void MainWindow::closeEvent(QCloseEvent *p_event)
{
	if (m_preview)
	{
		m_preview->preview_shutdown();
	}

	QMainWindow::closeEvent(p_event);
}

void MainWindow::showEvent(QShowEvent *p_event)
{
	if (ui->cbEnablePreview->isChecked())
	{
		show_preview();
	}

	QMainWindow::showEvent(p_event);
}

void MainWindow::on_selResolution_currentIndexChanged (int p_index)
{
	if (m_preview && !m_updating_ui)
	{
		auto &f_res = m_preview->video_resolutions()[p_index];
		
		ui->widget->setMinimumSize(f_res.m_width, f_res.m_height);
		ui->widget->setMaximumSize(f_res.m_width, f_res.m_height);
		ui->widget->updateGeometry();

		m_preview->video_window_resize({ui->widget->x(), ui->widget->y(), f_res.m_width, f_res.m_height});


		m_preview->video_change_resolution(p_index);
	}
}

void MainWindow::on_cbTracking_stateChanged (int p_state)
{
	if (ui->cbTracking->isChecked() != settings::TrackingEnabled)
		ui_to_settings();
}

void MainWindow::on_cbKinectV2_stateChanged (int p_state)
{
	if (ui->cbKinectV2->isChecked() != settings::KinectV2Enabled)
		ui_to_settings();
}

void MainWindow::on_cbKinectV1_stateChanged (int p_state)
{
	if (ui->cbKinectV1->isChecked() != settings::KinectV1Enabled)
		ui_to_settings();
}

void MainWindow::on_selTrackingJoint_currentIndexChanged (int p_index)
{
	if (p_index != settings::TrackingJoint)
		ui_to_settings();
}

void MainWindow::on_cbEnablePreview_stateChanged (int p_state)
{
	if (p_state == Qt::Checked)
		show_preview();
	else
		stop_preview();
}

void MainWindow::on_btnRegister_clicked()
{
	register_com_dll(L"kinect_webcam.ax");
}

void MainWindow::on_btnUnregister_clicked()
{
	unregister_com_dll(L"kinect_webcam.ax");
}

///////////////////////////////////////////////////////////////////////////////
//
// helper functions
//

void MainWindow::ui_from_settings()
{
	m_updating_ui = true;

	// effects - tracking
	ui->cbTracking->setChecked(settings::TrackingEnabled);
	ui->selTrackingJoint->setCurrentIndex(settings::TrackingJoint);

	// advanced - devices
	ui->cbKinectV2->setChecked(settings::KinectV2Enabled);
	ui->cbKinectV1->setChecked(settings::KinectV1Enabled);

	m_updating_ui = false;
}

void MainWindow::ui_to_settings()
{
	if (m_updating_ui)
		return;

	// effects - tracking
	settings::TrackingEnabled = ui->cbTracking->isChecked();
	settings::TrackingJoint	  = ui->selTrackingJoint->currentIndex();

	// advanced - devices
	settings::KinectV2Enabled = ui->cbKinectV2->isChecked();
	settings::KinectV1Enabled = ui->cbKinectV1->isChecked();

	// save to the registry
	settings::save();
}

void MainWindow::init_device()
{
	m_device_ok = m_preview->initialize(CLSID_KinectWebCam);

	if (m_device_ok)
	{
		m_updating_ui = true;

		ui->selResolution->clear();

		for (const auto &f_res: m_preview->video_resolutions())
		{
			ui->selResolution->addItem(QString("%1: %2 x %3 x %4").arg(f_res.m_id).arg(f_res.m_width).arg(f_res.m_height).arg(f_res.m_bpp));
		}

		m_updating_ui = false;
	}
}

void MainWindow::show_preview()
{
	if (m_preview)
	{
		m_preview->preview_device({ui->widget->x(), ui->widget->y(), ui->widget->width(), ui->widget->height()}, hwnd_from_widget(ui->widget));
	}
}

void MainWindow::stop_preview()
{
	if (m_preview)
	{
		m_preview->preview_shutdown();
	}
}