HEADERS += \
    transformation.h \
    objectmodel.h \
    material.h \
    mainwindow.h \
    light.h \
    glviewport.h \
    triangle.h \
    vertex.h \
    surreader.h \
    objreader.h

SOURCES += \
    transformation.cpp \
    objectmodel.cpp \
    material.cpp \
    mainwindow.cpp \
    main.cpp \
    light.cpp \
    glviewport.cpp \
    triangle.cpp \
    vertex.cpp \
    surreader.cpp \
    objreader.cpp

FORMS += \
    mainwindow.ui

RESOURCES += \
    icons.qrc

QT += opengl
