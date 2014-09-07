///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	mainwindow.h
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

#ifndef KW_MAINWINDOW_H
#define KW_MAINWINDOW_H

#include <QMainWindow>
#include <memory>

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

	public:
	   explicit MainWindow(QWidget *p_parent = 0);
		~MainWindow();

	private slots:
		void on_selResolution_currentIndexChanged (int p_index);
		void on_cbTracking_stateChanged (int p_state);
		void on_cbKinectV2_stateChanged (int p_state);
		void on_cbKinectV1_stateChanged (int p_state);
		void on_selTrackingJoint_currentIndexChanged (int p_index);

		void on_btnRegister_clicked();
		void on_btnUnregister_clicked();

	protected:
		void resizeEvent(QResizeEvent *p_event);
		void closeEvent(QCloseEvent *p_event);
		void showEvent(QShowEvent *p_event);

	private:
		void ui_from_settings();
		void ui_to_settings();

	private:
		Ui::MainWindow *ui;
		std::unique_ptr<class DSVideoCapture>	m_preview;
		bool									m_updating_ui;
};

#endif // KW_MAINWINDOW_H