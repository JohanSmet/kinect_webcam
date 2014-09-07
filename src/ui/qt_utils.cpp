///////////////////////////////////////////////////////////////////////////////
//
// File 	: 	qt_utils.cpp
//
// Purpose	: 	utility functions for QT
//
// Copyright (c) 2014	Contributors as noted in the AUTHORS file
//
// This file is licensed under the terms of the MIT license,
// for more details please see LICENSE.txt in the root directory
// of the provided source or http://opensource.org/licenses/MIT
//
///////////////////////////////////////////////////////////////////////////////

#include "qt_utils.h"
#include <qpa/qplatformnativeinterface.h>
#include <qwindow.h>
#include <qguiapplication.h>

static inline QWindow *window_from_widget(const QWidget *p_widget)
{
    QWindow* f_window = p_widget->windowHandle();

    if (f_window)
        return f_window;

    const QWidget* f_native_parent = p_widget->nativeParentWidget();

    if (f_native_parent)
        return f_native_parent->windowHandle();

    return nullptr;
}

HWND hwnd_from_widget(const QWidget *widget)
{
    QWindow * window = ::window_from_widget(widget);

    if (window && window->handle())
    {
        QPlatformNativeInterface* interface = QGuiApplication::platformNativeInterface();
        return static_cast<HWND>(interface->nativeResourceForWindow(QByteArrayLiteral("handle"), window));
    }
    return 0;
}
