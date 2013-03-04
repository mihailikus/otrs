#-------------------------------------------------
#
# Project created by QtCreator 2012-12-31T10:12:14
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += sql
QMAKE_CXXFLAGS += -fno-show-column

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OTRS
TEMPLATE = app


SOURCES += main.cpp\
        otrs.cpp \
    checker.cpp \
    OtrsModule.cpp \
    billingModule.cpp \
    timeChecker.cpp \
    OtrsWorker.cpp \
    answer.cpp \
    wizard.cpp

HEADERS  += otrs.h \
    ticket.h \
    login_config.h \
    checker.h \
    OtrsModule.h \
    billingModule.h \
    timeChecker.h \
    mysql_config.h \
    OtrsWorker.h \
    answer.h \
    wizard.h

FORMS    += \
    answer.ui \
    wizard.ui

OTHER_FILES +=

RESOURCES += \
    resource.qrc
