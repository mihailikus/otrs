#-------------------------------------------------
#
# Project created by QtCreator 2012-12-31T10:12:14
#
#-------------------------------------------------

QT       += core gui
QT  += network

QMAKE_CXXFLAGS += -fno-show-column

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OTRS
TEMPLATE = app


SOURCES += main.cpp\
        otrs.cpp \
    checker.cpp \
    OtrsModule.cpp \
    billingModule.cpp

HEADERS  += otrs.h \
    ticket.h \
    login_config.h \
    checker.h \
    OtrsModule.h \
    billingModule.h

FORMS    +=

OTHER_FILES +=

RESOURCES += \
    resource.qrc
