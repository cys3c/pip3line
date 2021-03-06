# Released as open source by Gabriel Caudrelier
#
# Developed by Gabriel Caudrelier, gabriel dot caudrelier at gmail dot com
#
# https://github.com/metrodango/pip3line
#
# Released under AGPL see LICENSE for more information

QT       += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = lib
CONFIG += plugin debug no_keywords c++11 warn_on

# Comment this out if you want to compile the 2.7 version
CONFIG += CONF_PYTHON_3
#DEFINES += CONF_PYTHON_27

CONF_PYTHON_3 {
    DEFINES += BUILD_PYTHON_3
    TARGET = python3plugin
    unix:!macx {
        LIBS += -lpython3.4
        INCLUDEPATH +="/usr/include/python3.4m/"
    }

    macx {
        LIBS += -L"/usr/local/Cellar/python3/3.5.1/Frameworks/Python.framework/Versions/3.5/lib/" -lpython3.5
        INCLUDEPATH +="/usr/local/Cellar/python3/3.5.1/Frameworks/Python.framework/Versions/3.5/include/python3.5m/" INCLUDEPATH
    }

    win32 {
        INCLUDEPATH +="C:\\Python34\\include\\"
        LIBS += -L"C:\\Python34\\libs\\" -lpython34
    }
} else {
    TARGET = python27plugin
    unix {
        LIBS += -lpython2.7
        INCLUDEPATH ="/usr/include/python2.7/" INCLUDEPATH
    }

    win32 {
        INCLUDEPATH +="C:\\Python27\\include\\"
        LIBS += -L"C:\\Python27\\libs\\" -lpython27
    }
}

unix {
    LIBS += -L"../../bin/" -ltransform
}

win32 {
    LIBS += -L"../../lib/" -ltransform
}

INCLUDEPATH += ../../libtransform/
DESTDIR = ../../bin/plugins

DEFINES += PYTHONPLUGIN_LIBRARY

SOURCES += pythonplugin.cpp \
    pythontransform.cpp \
    pythonmodules.cpp

HEADERS += pythonplugin.h\
        pythonplugin_global.h \
    pythontransform.h \
    pythonmodules.h

OTHER_FILES += \
    pythonplugin.json

