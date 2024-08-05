QT += quick location positioning
CONFIG += c++11


QML_IMPORT_PATH +=  \
    $$PWD/src \

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/MapTileProvider.cpp \

INCLUDEPATH += \
    inc \
    external \

HEADERS += \
    src/MapTileProvider.h \

RESOURCES += \
    resources.qrc


# Additional import path used to resolve QML modules in Qt Creator's code model

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
