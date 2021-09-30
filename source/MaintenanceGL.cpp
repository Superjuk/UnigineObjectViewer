/* Copyright (C) 2005-2018, UNIGINE. All rights reserved.
 *
 * This file is a part of the UNIGINE 2.7.2.1 SDK.
 *
 * Your use and / or redistribution of this software in source and / or
 * binary form, with or without modification, is subject to: (i) your
 * ongoing acceptance of and compliance with the terms and conditions of
 * the UNIGINE License Agreement; and (ii) your inclusion of this notice
 * in any version of this software that you use or redistribute.
 * A copy of the UNIGINE License Agreement is available by contacting
 * UNIGINE. at http://unigine.com/
 */


#include "MaintenanceGL.h"

#ifdef _LINUX
#include <GL/glx.h>
#include <GL/glxext.h>
#endif

MaintenanceGL::MaintenanceGL(QWidget *parent, Qt::WindowFlags flags)
	: Maintenance(parent, flags)
{
	context = initGLContext();
	setTitle("GLAppQt");
}

MaintenanceGL::~MaintenanceGL()
{
}

void *MaintenanceGL::getHandle()
{
#ifdef _WIN32
	return (void *)winId();
#elif _LINUX
	static Window window;
	window = winId();
	return &window;
#endif
}

const char *MaintenanceGL::getName()
{
	return "opengl";
}
