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


#include <QClipboard>
#include <QLayout>
#include <QWindow>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QDebug>
#include <QDir>
#include <QFileInfoList>
#include <clocale>

#include <UnigineEngine.h>

#include "Maintenance.h"

#ifdef _WIN32
#include <windows.h>
#pragma comment(lib, "user32.lib")
#endif

#ifdef _LINUX
	#include <GL/glx.h>
	#include <GL/glxext.h>
#endif

using namespace std;
using namespace Unigine;

class App;

Maintenance::Maintenance(QWidget *parent, Qt::WindowFlags flags)
	: QWidget(parent, flags)
{
	setlocale(LC_NUMERIC, "C");

    timer_resize = 1;
	timer_update = startTimer(0);

	window_width = 0;
	window_height = 0;
	window_flags = 0;
	window_frame = 0;
	stop_fps = 0;

	mouse_x = 0;
	mouse_y = 0;
	memset(mouse_axes, 0, sizeof(mouse_axes));
	mouse_button = 0;
	mouse_release = 0;
	mouse_grab = 0;
	mouse_show = 1;
	window_update = 1;

	memset(keys, 0, sizeof(keys));

#ifdef _WIN32
	setAttribute(Qt::WA_PaintOnScreen);
#endif
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_NativeWindow);
	setFocusPolicy(Qt::StrongFocus);

	qApp->installEventFilter(this);

	connect(qApp, &QApplication::applicationStateChanged, [this]() {
		if (qApp->applicationState() == Qt::ApplicationActive || this->getUpdate() == 1)
		{
			if (timer_update == -1)
				timer_update = startTimer(0);
		} else
		{
			killTimer(timer_update);
			timer_update = -1;
		}
    });    
}

Maintenance::~Maintenance()
{
}

void Maintenance::initEngine(QWidget *main_window, const char *dataPath)
{
    setParent(main_window);

    QVector<char*> argv;

    char arg_0[] = "-extern_define";
    argv.append(arg_0);
    char arg_1[] = "MOUSE_SOFT";
    argv.append(arg_1);

    char arg_2[] = "-video_mode";
    argv.append(arg_2);
    char arg_3[] = "-1";
    argv.append(arg_3);

    char arg_4[] = "-video_resizable";
    argv.append(arg_4);
    char arg_5[] = "1";
    argv.append(arg_5);

    char arg_6[] = "-video_fullscreen";
    argv.append(arg_6);
    char arg_7[] = "0";
    argv.append(arg_7);

    char arg_8[] = "-video_width";
    argv.append(arg_8);
    std::string width_str = std::to_string(this->size().width());
    char*       arg_9     = const_cast<char*>(width_str.c_str());
    argv.append(arg_9);

    char arg_10[] = "-video_height";
    argv.append(arg_10);
    std::string height_str = std::to_string(this->size().height());
    char*       arg_11     = const_cast<char*>(height_str.c_str());
    argv.append(arg_11);

    char arg_12[] = "-data_path";
    argv.append(arg_12);
    char* arg_13 = const_cast<char*>(dataPath);
    argv.append(arg_13);

    char arg_14[] = "-video_resizable";
    argv.append(arg_14);
    char* arg_15 = const_cast<char*>("1");
    argv.append(arg_15);

    char arg_16[] = "-engine_log";
    argv.append(arg_16);
    char* arg_17 = const_cast<char*>("../log/maintenance_log.html");
    argv.append(arg_17);

    char arg_18[] = "-sound_app";
    argv.append(arg_18);
    char* arg_19 = const_cast<char*>("null");
    argv.append(arg_19);

    char arg_20[] = "-extern_plugin";
    argv.append(arg_20);
    char* arg_21 = const_cast<char*>("FbxImporter,CadImporter");
    argv.append(arg_21);

    char arg_22[] = "-console_command";
    argv.append(arg_22);
    char* arg_23 = const_cast<char*>("world_load \"maintenance\"");
    argv.append(arg_23);
    argv.append(nullptr);


    Unigine::EnginePtr engine(UNIGINE_VERSION, this, argv.size(), argv.data());
    engine.release();
    _engine = engine;
    _engine.grab();

    _engine->addSystemLogic(&_systemLogic);
    _engine->addWorldLogic(&_worldLogic);
    _engine->addEditorLogic(&_editorLogic);

    connect(&_worldLogic, &MaintenanceWorldLogic::partSelected, this, &Maintenance::partSelected);
    connect(&_worldLogic, &MaintenanceWorldLogic::initComplete, this, &Maintenance::initComplete);
    connect(&_worldLogic, &MaintenanceWorldLogic::importComplete, this, &Maintenance::importComplete);
}

void Maintenance::setSize(int width, int height)
{
    auto app = App::get();

    app->setHeight(height);
    app->setWidth(width);
}

QVariantList Maintenance::getParts(const QString& modelPath)
{
    return _worldLogic.getParts(modelPath.toStdString().c_str());
}

QVariantList Maintenance::getInitParts()
{
    return _worldLogic.getInitParts();
}

void Maintenance::viewPart(int partId)
{
    _worldLogic.viewPart(partId);
}

void Maintenance::importModel(const QString &path)
{
    _worldLogic.importModel(String(path.toStdString().c_str()));
}

void Maintenance::setPartVisible(int partId, int state)
{
    _worldLogic.setPartVisible(partId, state);
}

bool Maintenance::setPartName(int partId, const QString &name)
{
    return _worldLogic.setPartName(partId, String(name.toStdString().c_str()));
}

void Maintenance::savePartNames()
{
    _worldLogic.savePartNamesToNode();
}

void Maintenance::shutdown()
{
    _worldLogic.shutdown();
}

//std::pair<const char*, const char*> Maintenance::getDescription(const char *model, const char *part)
//{
//    return make_pair("", "");
//}

//////////////////////////////////////////////////////////////////////////
// Events
//////////////////////////////////////////////////////////////////////////

QPaintEngine *Maintenance::paintEngine() const
{
	return NULL;
}

bool Maintenance::eventFilter(QObject *obj, QEvent *event)
{
	Q_UNUSED(obj);

	if (event->type() == QEvent::MouseMove)
		mouseMoveEvent(static_cast<QMouseEvent *>(event));

	return false;
}

void Maintenance::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == timer_update)
		update_engine();

	QWidget::timerEvent(event);
}

void Maintenance::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);

	timer_resize = 1;
	if (Unigine::Engine::isInitialized())
	{
		int width = size().width();
		int height = size().height();
		if (window_width != width || window_height != height)
		{
			window_width = width;
			window_height = height;
			resize_window();
		}
	}
}

void Maintenance::mousePressEvent(QMouseEvent *event)
{
	if (event->button() & Qt::LeftButton)
	{
		mouse_button |= BUTTON_LEFT;
		buttonPress(BUTTON_LEFT);
	}
	if (event->button() & Qt::MidButton)
	{
		mouse_button |= BUTTON_MIDDLE;
		buttonPress(BUTTON_MIDDLE);
	}
	if (event->button() & Qt::RightButton)
	{
		mouse_button |= BUTTON_RIGHT;
		buttonPress(BUTTON_RIGHT);
	}
	QWidget::mousePressEvent(event);
}

void Maintenance::mouseReleaseEvent(QMouseEvent *event)
{
	if ((event->button() & Qt::LeftButton) && (mouse_button & BUTTON_LEFT))
	{
		mouse_release |= BUTTON_LEFT;
		buttonRelease(BUTTON_LEFT);
	}
	if ((event->button() & Qt::MidButton) && (mouse_button & BUTTON_MIDDLE))
	{
		mouse_release |= BUTTON_MIDDLE;
		buttonRelease(BUTTON_MIDDLE);
	}
	if ((event->button() & Qt::RightButton) && (mouse_button & BUTTON_RIGHT))
	{
		mouse_release |= BUTTON_RIGHT;
		buttonRelease(BUTTON_RIGHT);
	}
	QWidget::mouseReleaseEvent(event);
}

void Maintenance::mouseDoubleClickEvent(QMouseEvent *event)
{
	Q_UNUSED(event);
	mouse_button |= BUTTON_DCLICK;
	mouse_release |= BUTTON_LEFT;
	buttonPress(BUTTON_DCLICK);
}

void Maintenance::wheelEvent(QWheelEvent *event)
{
	if (event->orientation() == Qt::Horizontal)
		mouse_axes[AXIS_X] += event->delta() / 120;
	else
		mouse_axes[AXIS_Y] += event->delta() / 120;
	QWidget::wheelEvent(event);
}

void Maintenance::mouseMoveEvent(QMouseEvent *event)
{
	QPoint pos = mapFromGlobal(event->globalPos());
	mouse_x = pos.x();
	mouse_y = pos.y();
	QWidget::mouseMoveEvent(event);
}

void Maintenance::update_engine()
{
#ifdef _LINUX
	QApplication::processEvents(QEventLoop::DialogExec);
#endif

	mouse_button &= ~mouse_release;
	mouse_release = 0;

	// update engine
	if (Unigine::Engine::isInitialized() && timer_resize == 0)
	{
		doUpdate();
		doRender();
		doSwap();
	}

	timer_resize = 0;

	// update widget
	QWidget::update();

	// clear mouse axes
	for (int i = 0; i < NUM_AXES; i++)
		mouse_axes[i] = 0;

	// release mouse button
	if (mouse_button & BUTTON_DCLICK)
	{
		mouse_button &= ~BUTTON_DCLICK;
		buttonRelease(BUTTON_DCLICK);
	}

	if (qApp->applicationState() == Qt::ApplicationActive)
	{
		Qt::KeyboardModifiers modifiers = qApp->queryKeyboardModifiers();
		if ((modifiers & Qt::ShiftModifier))
		{
			keys[KEY_SHIFT] = 1;
			keyPress(KEY_SHIFT);
		} else
		{
			keys[KEY_SHIFT] = 0;
			keyRelease(KEY_SHIFT);
		}
		if ((modifiers & Qt::AltModifier))
		{
			keys[KEY_ALT] = 1;
			keyPress(KEY_ALT);
		} else
		{
			keys[KEY_ALT] = 0;
			keyRelease(KEY_ALT);
		}
		unsigned int control_key = KEY_CTRL;
#ifdef _MACOS
		control_key = KEY_CMD;
		if ((modifiers & Qt::MetaModifier))
		{
			keys[KEY_CTRL] = 1;
			keyPress(KEY_CTRL);
		} else
		{
			keys[KEY_CTRL] = 0;
			keyRelease(KEY_CTRL);
		}
#endif
		if ((modifiers & Qt::ControlModifier))
		{
			keys[control_key] = 1;
			keyPress(control_key);
		} else
		{
			keys[control_key] = 0;
			keyRelease(control_key);
		}
	}

#ifdef _MACOS
	if (this->rect().contains(this->mapFromGlobal(this->cursor().pos())))
	{
		if (!mouse_show && parentWidget()->cursor().shape() != Qt::BlankCursor)
			parentWidget()->setCursor(QCursor(Qt::BlankCursor));
	} else
		parentWidget()->unsetCursor();
#endif
}

void Maintenance::keyPressEvent(QKeyEvent *event)
{
	unsigned int key = translate_key_from_qt(event->key());
	if (key == 0)
		key = translate_native_key_from_event(event);

	if (key < NUM_KEYS)
		keys[key] = 1;
	if (key >= KEY_ESC && key <= KEY_F12)
		keyPressUnicode(key);
	else
	{
		const QChar *s = event->text().unicode();
		if (s)
			keyPressUnicode(s->unicode());
	}
	if (key)
		keyPress(key);
	QWidget::keyPressEvent(event);
}

void Maintenance::keyReleaseEvent(QKeyEvent *event)
{
	unsigned int key = translate_key_from_qt(event->key());
	if (key == 0)
		key = translate_native_key_from_event(event);

	if (key < NUM_KEYS)
		keys[key] = 0;
	if (key)
		keyRelease(key);
	QWidget::keyReleaseEvent(event);
}

void Maintenance::focusInEvent(QFocusEvent *event)
{
	QWidget::focusInEvent(event);
}

void Maintenance::focusOutEvent(QFocusEvent *event)
{
	window_frame = 0;

	// release keys if switched to non interface window
	QWindow *obj = qApp->focusWindow();
	if (obj && obj->objectName() != "interface window")
	{
		for (int i = 0; i < NUM_KEYS; i++)
		{
			if (keys[i])
			{
				keyRelease(i);
				keys[i] = 0;
			}
		}
	}

	QWidget::focusOutEvent(event);
}

//////////////////////////////////////////////////////////////////////////
// App interface
//////////////////////////////////////////////////////////////////////////

void *Maintenance::getHandle()
{
	return (void *)winId();
}

int Maintenance::setPosition(int x, int y)
{
	if (parent())
		parentWidget()->move(mapFromParent({x, y}));
	else
		move(mapFromGlobal({x, y}));
	return 0;
}

int Maintenance::getPositionX()
{
	return mapToGlobal({0, 0}).x();
}

int Maintenance::getPositionY()
{
	return mapToGlobal({0, 0}).y();
}

int Maintenance::setVideoMode(int width, int height, int flags, int refresh)
{
	Q_UNUSED(refresh);

	window_width = width;
	window_height = height;
    window_flags = flags;
	Unigine::Log::message("Set %s video mode\n", getVideoModeName());

    setMinimumSize(10, 10);
	setMaximumSize(1000000, 1000000);

	resize(width, height);

	QCoreApplication::processEvents();

	create_context();

	return 1;
}

int Maintenance::restoreVideoMode()
{
	return destroy_context();
}

int Maintenance::setUpdate(int update)
{
	window_update = update;
	return 1;
}

int Maintenance::setGamma(float value)
{
	Q_UNUSED(value);
	return 1;
}

int Maintenance::setTitle(const char *title)
{
	QString t = title;

	if (parent())
	{
		QWidget *widget = qobject_cast<QWidget *>(parent());
		widget->setWindowTitle(t);
	} else
		setWindowTitle(t);
	return 1;
}

int Maintenance::setIcon(const unsigned char *data, int size)
{
	Q_UNUSED(data);
	Q_UNUSED(size);
	return 0;
}

int Maintenance::getWidth()
{
	return window_width;
}

int Maintenance::getHeight()
{
	return window_height;
}

void Maintenance::setWidth(int value)
{
	Q_UNUSED(value);
}

void Maintenance::setHeight(int value)
{
	Q_UNUSED(value);
}

int Maintenance::getFlags()
{
	return window_flags;
}

int Maintenance::getUpdate()
{
	return window_update;
}

void Maintenance::setMouse(int x, int y)
{
	mouse_x = x;
	mouse_y = y;
	QCursor::setPos(mapToGlobal(QPoint(x, y)));
}

void Maintenance::setMouseGrab(int grab)
{
	if (mouse_grab == grab)
		return;
	mouse_grab = grab;
	if (grab)
		grabMouse();
	else
		releaseMouse();
}

void Maintenance::setMouseShow(int show)
{
	if (mouse_show == show)
		return;
	mouse_show = show;
	if (show)
		setCursor(QCursor(Qt::ArrowCursor));
	else
		setCursor(QCursor(Qt::BlankCursor));
}

int Maintenance::setMouseCursor(const unsigned char *data, int size, int x, int y)
{
	Q_UNUSED(data);
	Q_UNUSED(size);
	Q_UNUSED(x);
	Q_UNUSED(y);
	return 0;
}

int Maintenance::getMouseX()
{
	return mouse_x;
}

int Maintenance::getMouseY()
{
	return mouse_y;
}

int Maintenance::getMouseGrab()
{
	return mouse_grab;
}

int Maintenance::getMouseShow()
{
	return mouse_show;
}

void Maintenance::setMouseAxis(int axis, int value)
{
	if (axis < 0 || axis >= NUM_AXES)
		return;
	mouse_axes[axis] = value;
}

int Maintenance::getMouseAxis(int axis)
{
	if (axis < 0 || axis >= NUM_AXES)
		return 0;
	return mouse_axes[axis];
}

int Maintenance::clearMouseAxis(int axis)
{
	if (axis < 0 || axis >= NUM_AXES)
		return 0;
	int ret = mouse_axes[axis];
	mouse_axes[axis] = 0;
	return ret;
}

void Maintenance::setMouseButton(int button)
{
	mouse_button = button;
}

int Maintenance::getMouseButton()
{
	return mouse_button;
}

int Maintenance::getMouseButtonState(int button)
{
	return ((mouse_button & button) != 0);
}

int Maintenance::clearMouseButtonState(int button)
{
	int ret = ((mouse_button & button) != 0);
	mouse_button &= ~button;
	return ret;
}

int Maintenance::getNumTouches()
{
	return 0;
}

int Maintenance::getTouchX(int touch)
{
	Q_UNUSED(touch);
	return -1000000;
}

int Maintenance::getTouchY(int touch)
{
	Q_UNUSED(touch);
	return -1000000;
}

void Maintenance::setKeyState(int key, int state)
{
	if (key < 0 || key >= NUM_KEYS)
		return;
	keys[key] = state;
}

int Maintenance::getKeyState(int key)
{
	if (key < 0 || key >= NUM_KEYS)
		return 0;
	return keys[key];
}

int Maintenance::clearKeyState(int key)
{
	if (key < 0 || key >= NUM_KEYS)
		return 0;
	int ret = keys[key];
	keys[key] = 0;
	return ret;
}

void Maintenance::setClipboard(const char *str)
{
	QApplication::clipboard()->setText(str);
}

const char *Maintenance::getClipboard()
{
	static char clipboard[4096] = {0};

	const QByteArray &utf8_str = QApplication::clipboard()->text().toUtf8();
	const int size = qMin(4095, utf8_str.size());

	strncpy(clipboard, utf8_str.data(), size);
	clipboard[size] = '\0';

	return clipboard;
}

int Maintenance::isDone()
{
	return 0;
}

void Maintenance::doUpdate()
{
	App::update();
}

void Maintenance::doRender()
{
	context->renderWindow();
	App::render();
}

void Maintenance::doSwap()
{
	App::swap();
	swap_window();
}

void Maintenance::exit()
{
	close();
	closed();
	QCoreApplication::quit();
}

int Maintenance::dialogMessage(const char *title, const char *str, const char *flags)
{
	Q_UNUSED(flags);
	int ret = QMessageBox::warning(this, title, str, QMessageBox::Ok, QMessageBox::Cancel);
	return (ret == QMessageBox::Ok);
}

int Maintenance::dialogFile(const char *title, char *name, int size, const char *filter, const char *flags)
{
	Q_UNUSED(flags);
	Q_UNUSED(filter);
	QString str = QFileDialog::getOpenFileName(this, title, name, QString::null, 0);
	strncpy(name, str.toUtf8().data(), size);
	name[size - 1] = '\0';
	return 1;
}

int Maintenance::create_context()
{
	if (!context->isReady())
	{
		App::destroy();
		context->destroyContext();
		context->createVisual();
#ifdef _LINUX
		Window w = windowHandle()->winId();
		context->createContext(&w, getWidth(), getHeight());
#else
		context->createContext((void*)winId(), getWidth(), getHeight());
#endif
	} else {
		context->resizeWindow(getWidth(), getHeight());
	}

	return 1;
}

int Maintenance::destroy_context()
{
	context->destroyContext();
	return 1;
}

int Maintenance::resize_window()
{
	if (context)
		context->resizeWindow(getWidth(), getHeight());
	return 1;
}

int Maintenance::swap_window()
{
	context->swapWindow();
	return 1;
}


unsigned int Maintenance::translate_key_from_qt(int key) const
{
	using namespace Unigine;
	unsigned int ret = 0;
	switch (key)
	{
		case Qt::Key_Escape: ret = App::KEY_ESC; break;
		case Qt::Key_Tab: ret = App::KEY_TAB; break;
		case Qt::Key_Backspace: ret = App::KEY_BACKSPACE; break;
		case Qt::Key_Return: ret = App::KEY_RETURN; break;
		case Qt::Key_Enter: ret = App::KEY_RETURN; break;
		case Qt::Key_Delete: ret = App::KEY_DELETE; break;
		case Qt::Key_Insert: ret = App::KEY_INSERT; break;
		case Qt::Key_Home: ret = App::KEY_HOME; break;
		case Qt::Key_End: ret = App::KEY_END; break;
		case Qt::Key_PageUp: ret = App::KEY_PGUP; break;
		case Qt::Key_PageDown: ret = App::KEY_PGDOWN; break;
		case Qt::Key_Left: ret = App::KEY_LEFT; break;
		case Qt::Key_Right: ret = App::KEY_RIGHT; break;
		case Qt::Key_Up: ret = App::KEY_UP; break;
		case Qt::Key_Down: ret = App::KEY_DOWN; break;
		case Qt::Key_Shift: ret = App::KEY_SHIFT; break;
		case Qt::Key_Control: ret = App::KEY_CTRL; break;
		case Qt::Key_Alt: ret = App::KEY_ALT; break;
		case Qt::Key_F1: ret = App::KEY_F1; break;
		case Qt::Key_F2: ret = App::KEY_F2; break;
		case Qt::Key_F3: ret = App::KEY_F3; break;
		case Qt::Key_F4: ret = App::KEY_F4; break;
		case Qt::Key_F5: ret = App::KEY_F5; break;
		case Qt::Key_F6: ret = App::KEY_F6; break;
		case Qt::Key_F7: ret = App::KEY_F7; break;
		case Qt::Key_F8: ret = App::KEY_F8; break;
		case Qt::Key_F9: ret = App::KEY_F9; break;
		case Qt::Key_F10: ret = App::KEY_F10; break;
		case Qt::Key_F11: ret = App::KEY_F11; break;
		case Qt::Key_F12: ret = App::KEY_F12; break;
		default:
		{
			if (key < App::NUM_KEYS)
			{
				ret = key;
				if (ret >= 'A' && ret <= 'Z')
					ret -= 'A' - 'a';
				else if (ret == '!')
					ret = '1';
				else if (ret == '@')
					ret = '2';
				else if (ret == '#')
					ret = '3';
				else if (ret == '$')
					ret = '4';
				else if (ret == '%')
					ret = '5';
				else if (ret == '^')
					ret = '6';
				else if (ret == '&')
					ret = '7';
				else if (ret == '*')
					ret = '8';
				else if (ret == '(')
					ret = '9';
				else if (ret == ')')
					ret = '0';
				else if (ret == '_')
					ret = '-';
				else if (ret == '+')
					ret = '=';
				else if (ret == '{')
					ret = '[';
				else if (ret == '}')
					ret = ']';
				else if (ret == '|')
					ret = '\\';
				else if (ret == ':')
					ret = ';';
				else if (ret == '"')
					ret = '\'';
				else if (ret == '>')
					ret = '.';
				else if (ret == '?')
					ret = '/';
			}
		}
	}
	return ret;
}

int Maintenance::translate_key_to_qt(unsigned int key) const
{
	using namespace Unigine;
	int ret = 0;
	switch (key)
	{
		case App::KEY_ESC: ret = Qt::Key_Escape; break;
		case App::KEY_TAB: ret = Qt::Key_Tab; break;
		case App::KEY_BACKSPACE: ret = Qt::Key_Backspace; break;
		case App::KEY_RETURN: ret = Qt::Key_Return; break;
		case App::KEY_DELETE: ret = Qt::Key_Delete; break;
		case App::KEY_INSERT: ret = Qt::Key_Insert; break;
		case App::KEY_HOME: ret = Qt::Key_Home; break;
		case App::KEY_END: ret = Qt::Key_End; break;
		case App::KEY_PGUP: ret = Qt::Key_PageUp; break;
		case App::KEY_PGDOWN: ret = Qt::Key_PageDown; break;
		case App::KEY_LEFT: ret = Qt::Key_Left; break;
		case App::KEY_RIGHT: ret = Qt::Key_Right; break;
		case App::KEY_UP: ret = Qt::Key_Up; break;
		case App::KEY_DOWN: ret = Qt::Key_Down; break;
		case App::KEY_SHIFT: ret = Qt::Key_Shift; break;
		case App::KEY_CTRL: ret = Qt::Key_Control; break;
		case App::KEY_CMD: ret = Qt::Key_Meta; break;
		case App::KEY_ALT: ret = Qt::Key_Alt; break;
		case App::KEY_SCROLL: ret = Qt::Key_ScrollLock; break;
		case App::KEY_CAPS: ret = Qt::Key_CapsLock; break;
		case App::KEY_NUM: ret = Qt::Key_NumLock; break;
		case App::KEY_F1: ret = Qt::Key_F1; break;
		case App::KEY_F2: ret = Qt::Key_F2; break;
		case App::KEY_F3: ret = Qt::Key_F3; break;
		case App::KEY_F4: ret = Qt::Key_F4; break;
		case App::KEY_F5: ret = Qt::Key_F5; break;
		case App::KEY_F6: ret = Qt::Key_F6; break;
		case App::KEY_F7: ret = Qt::Key_F7; break;
		case App::KEY_F8: ret = Qt::Key_F8; break;
		case App::KEY_F9: ret = Qt::Key_F9; break;
		case App::KEY_F10: ret = Qt::Key_F10; break;
		case App::KEY_F11: ret = Qt::Key_F11; break;
		case App::KEY_F12: ret = Qt::Key_F12; break;
		default:
		{
			if (key < App::NUM_KEYS)
			{
				ret = key;
				if (ret >= 'a' && ret <= 'z')
					ret -= 'a' - 'A';
			}
		}
	}
	return ret;
}

unsigned int Maintenance::translate_native_key(unsigned int vkey) const
{
	using namespace Unigine;
	unsigned int ret = 0;
#ifdef _WIN32
	switch (vkey)
	{
		case VK_ESCAPE: ret = App::KEY_ESC; break;
		case VK_TAB: ret = App::KEY_TAB; break;
		case VK_RETURN: ret = App::KEY_RETURN; break;
		case VK_BACK: ret = App::KEY_BACKSPACE; break;
		case VK_DELETE: ret = App::KEY_DELETE; break;
		case VK_INSERT: ret = App::KEY_INSERT; break;
		case VK_HOME: ret = App::KEY_HOME; break;
		case VK_END: ret = App::KEY_END; break;
		case VK_PRIOR: ret = App::KEY_PGUP; break;
		case VK_NEXT: ret = App::KEY_PGDOWN; break;
		case VK_LEFT: ret = App::KEY_LEFT; break;
		case VK_RIGHT: ret = App::KEY_RIGHT; break;
		case VK_UP: ret = App::KEY_UP; break;
		case VK_DOWN: ret = App::KEY_DOWN; break;
		case VK_SHIFT: ret = App::KEY_SHIFT; break;
		case VK_CONTROL: ret = App::KEY_CTRL; break;
		case VK_MENU: ret = App::KEY_ALT; break;
		case VK_SCROLL: ret = App::KEY_SCROLL; break;
		case VK_CAPITAL: ret = App::KEY_CAPS; break;
		case VK_NUMLOCK: ret = App::KEY_NUM; break;
		case VK_F1: ret = App::KEY_F1; break;
		case VK_F2: ret = App::KEY_F2; break;
		case VK_F3: ret = App::KEY_F3; break;
		case VK_F4: ret = App::KEY_F4; break;
		case VK_F5: ret = App::KEY_F5; break;
		case VK_F6: ret = App::KEY_F6; break;
		case VK_F7: ret = App::KEY_F7; break;
		case VK_F8: ret = App::KEY_F8; break;
		case VK_F9: ret = App::KEY_F9; break;
		case VK_F10: ret = App::KEY_F10; break;
		case VK_F11: ret = App::KEY_F11; break;
		case VK_F12: ret = App::KEY_F12; break;
		case VK_LWIN: ret = App::KEY_CMD;
		case VK_RWIN: ret = App::KEY_CMD;
		default:
			ret = MapVirtualKeyW(vkey, 2);
			if (ret >= App::KEY_ESC)
				ret = 0;
			else if (ret >= 'A' && ret <= 'Z')
				ret -= (unsigned int)('A' - 'a');
	}
#elif _LINUX
	switch (vkey)
	{
		case 24: ret = 'q'; break;
		case 25: ret = 'w'; break;
		case 26: ret = 'e'; break;
		case 27: ret = 'r'; break;
		case 28: ret = 't'; break;
		case 29: ret = 'y'; break;
		case 30: ret = 'u'; break;
		case 31: ret = 'i'; break;
		case 32: ret = 'o'; break;
		case 33: ret = 'p'; break;
		case 34: ret = '['; break;
		case 35: ret = ']'; break;

		case 38: ret = 'a'; break;
		case 39: ret = 's'; break;
		case 40: ret = 'd'; break;
		case 41: ret = 'f'; break;
		case 42: ret = 'g'; break;
		case 43: ret = 'h'; break;
		case 44: ret = 'j'; break;
		case 45: ret = 'k'; break;
		case 46: ret = 'l'; break;
		case 47: ret = ';'; break;
		case 48: ret = '\''; break;
		case 49: ret = '`'; break;

		case 52: ret = 'z'; break;
		case 53: ret = 'x'; break;
		case 54: ret = 'c'; break;
		case 55: ret = 'v'; break;
		case 56: ret = 'b'; break;
		case 57: ret = 'n'; break;
		case 58: ret = 'm'; break;
		case 59: ret = ','; break;
		case 60: ret = '.'; break;
		case 61: ret = '/'; break;

		default: ret = 0;
	}
#endif
	return ret;
}

unsigned int Maintenance::translate_native_key_from_event(QKeyEvent *event) const
{
#ifndef _LINUX
	unsigned int vkey = event->nativeVirtualKey();
#else
	unsigned int vkey = event->nativeScanCode();
#endif

	return translate_native_key(vkey);
}

int Maintenance::translate_modifier_to_qt(unsigned int key) const
{
	int ret = 0;
	if ((key & (1 << 1)) != 0)
		ret += Qt::CTRL;
	if ((key & (1 << 0)) != 0)
		ret += Qt::SHIFT;
	if ((key & (1 << 2)) != 0)
        ret += Qt::ALT;
    return ret;
}
