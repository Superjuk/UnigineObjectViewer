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

#include "MaintenanceEditorLogic.h"
#include "MaintenanceSystemLogic.h"
#include "MaintenanceWorldLogic.h"

#include <QMainWindow>
#include <QtWidgets/QWidget>
#include <QStringList>

#include <UnigineApp.h>
#include <UnigineEngine.h>

#include <vector>

class Maintenance
        : public QWidget
        , public Unigine::App
{
	Q_OBJECT

public:
    Maintenance(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    virtual ~Maintenance();

    // visualizer widget
    void initEngine(QWidget* main_window, const char* dataPath);

    void setSize(int width, int height);

    //! get part list of selected model
    QVariantList getParts(const QString& modelPath);

    //! get initial part names of visible model
    QVariantList getInitParts();

    void viewPart(int partId);

    //! transmit path to model to world logic
    void importModel(const QString& path);

    //! set visible part status
    //! \arg state is Qt::CheckState
    void setPartVisible(int partId, int state);

    //! renames parts name, if it`s not empty
    bool setPartName(int partId, const QString& name);

    //! save part names changes immediately
    void savePartNames();

    //! shutdown world
    void shutdown();

    //! get description of part
    //! \return pair<title, description>
    //std::pair<const char *, const char *> getDescription(const char* model, const char* part);

protected:
	// window events
	virtual QPaintEngine *paintEngine() const;
	virtual void timerEvent(QTimerEvent *event);
	virtual void resizeEvent(QResizeEvent *event);

	// mouse events
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void wheelEvent(QWheelEvent *event);

	// keyboard events
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void keyReleaseEvent(QKeyEvent *event);

	virtual void focusInEvent(QFocusEvent *event);
	virtual void focusOutEvent(QFocusEvent *event);

	virtual bool eventFilter(QObject *obj, QEvent *event);

signals:
	void closed();

    void partSelected(int partID);

    void initComplete();

    void importComplete(const QString& modelName, const QVariantList& parts);

public:
	// app handle
	virtual void *getHandle();

	// window position
	virtual int setPosition(int x, int y);
	virtual int getPositionX();
	virtual int getPositionY();

	// video mode
	virtual int setVideoMode(int width, int height, int flags = 0, int refresh = 0);
	virtual int restoreVideoMode();
	virtual int setUpdate(int update);
	virtual int setGamma(float gamma);
	virtual int setTitle(const char *title);
	virtual int setIcon(const unsigned char *data, int size);
	virtual int getWidth();
	virtual int getHeight();
	virtual void setWidth(int value);
	virtual void setHeight(int value);
	virtual int getFlags();
	virtual int getUpdate();

	// mouse
	virtual void setMouse(int x, int y);
	virtual void setMouseGrab(int grab);
	virtual void setMouseShow(int show);
	virtual int setMouseCursor(const unsigned char *data, int size, int x = 0, int y = 0);
	virtual int getMouseX();
	virtual int getMouseY();
	virtual int getMouseGrab();
	virtual int getMouseShow();
	virtual void setMouseAxis(int axis, int value);
	virtual int getMouseAxis(int axis);
	virtual int clearMouseAxis(int axis);
	virtual void setMouseButton(int button);
	virtual int getMouseButton();
	virtual int getMouseButtonState(int button);
	virtual int clearMouseButtonState(int button);

	// touches
	virtual int getNumTouches();
	virtual int getTouchX(int touch);
	virtual int getTouchY(int touch);

	// keyboard
	virtual void setKeyState(int key, int state);
	virtual int getKeyState(int key);
	virtual int clearKeyState(int key);

	// clipboard
	virtual void setClipboard(const char *str);
	virtual const char *getClipboard();

	// main loop
	virtual int isDone();
	virtual void doUpdate();
	virtual void doRender();
	virtual void doSwap();
	virtual void exit();

	// dialogs
	virtual int dialogMessage(const char *title, const char *str, const char *flags = 0);
	virtual int dialogFile(const char *title, char *name, int size, const char *filter = 0, const char *flags = 0);

protected:
	Unigine::RenderContextPtr context;

private:
	void update_engine();

	virtual int create_context();
	virtual int destroy_context();
	virtual int resize_window();
	virtual int swap_window();

	// common
	unsigned int translate_key_from_qt(int key) const;
	int translate_key_to_qt(unsigned int key) const;
	unsigned int translate_native_key(unsigned int vkey) const;
	unsigned int translate_native_key_from_event(QKeyEvent *event) const;
	int translate_modifier_to_qt(unsigned int key) const;

	int timer_resize; // timer on resize event
	int timer_update; // timer on update engine event

	int window_width; // window
	int window_height;
	int window_flags;
	int window_update;
	int window_frame;
	int window_gamma;
	int stop_fps;

	int mouse_x; // mouse
	int mouse_y;
	int mouse_axes[NUM_AXES];
	int mouse_button;
	int mouse_release;
	int mouse_grab;
	int mouse_show;


	int keys[NUM_KEYS]; // keyboard

	QWidget *ignore_focus;
	QString engine_clipboard;

    Unigine::EnginePtr     _engine;
    MaintenanceSystemLogic _systemLogic;
    MaintenanceWorldLogic  _worldLogic;
    MaintenanceEditorLogic _editorLogic;

    QVariantList _models;  // list of models
};
