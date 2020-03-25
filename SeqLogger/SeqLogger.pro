QT -= gui

#CONFIG -= qt

TEMPLATE = lib
DEFINES += SEQLOGGER_LIBRARY

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    seqlogger.cpp

HEADERS += \
    SeqLogger_global.h \
    seqlogger.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -L$$PWD/../../../3rd-Party/curl-7.69.1/build-curl-7.69.1-Desktop_Qt_5_14_1_MinGW_64_bit-Release/lib/ -llibcurl_imp

INCLUDEPATH += $$PWD/../../../3rd-Party/curl-7.69.1/curl-7.69.1/include
DEPENDPATH += $$PWD/../../../3rd-Party/curl-7.69.1/curl-7.69.1/include

DISTFILES += \
    .astylerc