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

#pragma once

#include <Maintenance.h>

#include <UnigineApp.h>
#include <UnigineEngine.h>

class MaintenanceGL : public Maintenance
{
	Q_OBJECT

public:
    MaintenanceGL(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~MaintenanceGL();

	virtual void *getHandle();
	virtual const char *getName();
};
