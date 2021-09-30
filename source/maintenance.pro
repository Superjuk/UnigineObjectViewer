TARGET = maintenance
TEMPLATE = app
DESTDIR = ../bin
QT += core gui widgets
CONFIG += console c++14

UNGINE_SDK_PATH = ".."

MOC_DIR = moc
OBJECTS_DIR = obj
UI_DIR = ui
RCC_DIR = rcc

INCLUDEPATH += \
	AppQt \
        $$UNGINE_SDK_PATH/include \

HEADERS += \
    MainWindow.h \
    Maintenance.h \
    MaintenanceEditorLogic.h \
    MaintenanceGL.h \
    MaintenanceSystemLogic.h \
    MaintenanceWorldLogic.h \
    OfficeImport.h \
    ResizeControlWidget.h \
    TreeItem.h \
    TreeModel.h \
    defines.h \
    ViewCube.h

SOURCES += \
    Maintenance.cpp \
    MaintenanceEditorLogic.cpp \
    MaintenanceGL.cpp \
    MaintenanceSystemLogic.cpp \
    MaintenanceWorldLogic.cpp \
    OfficeImport.cpp \
    ResizeControlWidget.cpp \
    TreeItem.cpp \
    TreeModel.cpp \
    main.cpp \
    MainWindow.cpp \
    ViewCube.cpp

FORMS += MainWindow.ui

ARCH = _x64

IS_DEBUG = 
CONFIG(debug, debug|release) {
	IS_DEBUG = d
	DEFINES += DEBUG
} else {
	DEFINES += NDEBUG
}

BUILD_TYPE = $$join(ARCH,,,$$IS_DEBUG)
LIBS += -lUnigine_double$$BUILD_TYPE
TARGET = $$join(TARGET,,,$$BUILD_TYPE)

DEFINES += UNIGINE_DOUBLE

win32 {
	
	HEADERS +=
	SOURCES +=	
	DEFINES += _CRT_SECURE_NO_WARNINGS
	LIBS += -L$$UNGINE_SDK_PATH/lib
}

unix:!macx {
	
	DEFINES += _LINUX
	LIBS += -L$$UNGINE_SDK_PATH/bin
	LIBS += -lGL -lX11 -ldl -lXinerama
	QMAKE_LFLAGS += -Wl,-rpath,./
}
