QT       += core gui network sql opengl printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++2a

DEFINES *= QCUSTOMPLOT_USE_OPENGL

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aslogger.cpp \
    db.cpp \
    main.cpp \
    asupsdlg.cpp \
    qcustomplot.cpp

HEADERS += \
    asfx.h \
    aslogger.h \
    asupsdlg.h \
    db.h \
    qcustomplot.h

FORMS += \
    asupsdlg.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    .clang-format \
    .gitignore

RESOURCES += \
    resources.qrc
