HEADERS += \
    objectmodel.h \
    material.h \
    mainwindow.h \
    light.h \
    glviewport.h \
    polygon.h \
    vertex.h \
    surreader.h \
    objreader.h

SOURCES += \
    objectmodel.cpp \
    material.cpp \
    mainwindow.cpp \
    main.cpp \
    light.cpp \
    glviewport.cpp \
    polygon.cpp \
    vertex.cpp \
    surreader.cpp \
    objreader.cpp

FORMS += \
    mainwindow.ui

RESOURCES += \
    icons.qrc \
    textures.qrc

QT += opengl

mac: LIBS += -framework GLUT
unix:!macx|win32: LIBS += -lfreeglut
