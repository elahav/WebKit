/*
 * Copyright (C) 2014 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "PlatformDisplayQnxScreen.h"

#if PLATFORM(QNXSCREEN)

#include "GLContext.h"
#include <cstring>
#include <wtf/Assertions.h>
#include <epoxy/egl.h>

#include <gtk/gtk.h>

#if (USE_GTK4)
// FIXME: This is a GDK private function. Wayland exports a wrapper.
// Should qnxscreen do the same?
extern "C" {
    extern gpointer gdk_qnxscreen_display_get_egl_display(GdkDisplay *);
}
#endif

namespace WebCore {

std::unique_ptr<PlatformDisplay> PlatformDisplayQnxScreen::create(GdkDisplay* display)
{
    return makeUnique<PlatformDisplayQnxScreen>(display);
}

PlatformDisplayQnxScreen::PlatformDisplayQnxScreen(GdkDisplay* display)
    : PlatformDisplay(display)
{
}

PlatformDisplayQnxScreen::~PlatformDisplayQnxScreen()
{
}

void PlatformDisplayQnxScreen::sharedDisplayDidClose()
{
    PlatformDisplay::sharedDisplayDidClose();
}

EGLDisplay PlatformDisplayQnxScreen::gtkEGLDisplay()
{
    if (m_eglDisplay != EGL_NO_DISPLAY)
        return m_eglDisplayOwned ? EGL_NO_DISPLAY : m_eglDisplay;

#if USE(GTK4)
    // FIXME:
    // This code was copied from the wayland version, and it's not clear what it is doing and
    // why.
    //m_eglDisplay = gdk_qnxscreen_display_get_egl_display(m_sharedDisplay.get());
    m_eglDisplay = EGL_NO_DISPLAY;
#else
    auto* window = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_widget_realize(window);
    if (auto context = adoptGRef(gdk_window_create_gl_context(gtk_widget_get_window(window), nullptr))) {
        gdk_gl_context_make_current(context.get());
        m_eglDisplay = eglGetCurrentDisplay();
    }
    gtk_widget_destroy(window);
#endif

    if (m_eglDisplay == EGL_NO_DISPLAY)
        return EGL_NO_DISPLAY;

    m_eglDisplayOwned = false;
    PlatformDisplay::initializeEGLDisplay();
    return m_eglDisplay;
}

void PlatformDisplayQnxScreen::initializeEGLDisplay()
{
    if (gtkEGLDisplay() != EGL_NO_DISPLAY)
        return;

    if (m_eglDisplay == EGL_NO_DISPLAY)
        m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    PlatformDisplay::initializeEGLDisplay();
}

} // namespace WebCore

#endif // PLATFORM(QNXSCREEN)
