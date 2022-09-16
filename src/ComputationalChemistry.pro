QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -L../libs -lglew32 -lopengl32 -L"C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.1\lib\x64" -lopencl

INCLUDEPATH += ../includes

SOURCES += \
    common/atom.cpp \
    common/fileloading.cpp \
    common/parameterization.cpp \
    main.cpp \
    rendering/globalprogrammemanager.cpp \
    rendering/gridrenderer.cpp \
    rendering/linerenderer.cpp \
    rendering/sphererenderer.cpp \
    simulation/mdsimulation.cpp \
    simulation/scene.cpp \
    ui/floatinghintwidget.cpp \
    ui/lewisview.cpp \
    ui/licenses.cpp \
    ui/mainwindow.cpp \
    ui/moleculedetails.cpp \
    ui/moleculedialog.cpp \
    ui/renderwidget.cpp

HEADERS += \
    common/atom.h \
    common/data_elements.h \
    common/fileloading.h \
    common/parameterization.h \
    interfaces/iemitresize.h \
    rendering/globalprogrammemanager.h \
    rendering/gridrenderer.h \
    rendering/irenderer.h \
    rendering/linerenderer.h \
    rendering/sphererenderer.h \
    simulation/mdsimulation.h \
    simulation/scene.h \
    ui/floatinghintwidget.h \
    ui/lewisview.h \
    ui/licenses.h \
    ui/mainwindow.h \
    ui/moleculedetails.h \
    ui/moleculedialog.h \
    ui/renderwidget.h

FORMS += \
    ui/floatinghintwidget.ui \
    ui/licenses.ui \
    ui/mainwindow.ui \
    ui/moleculedetails.ui \
    ui/newmolecule.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    computation/mdkernel.cl \
    elementdata.txt \
    shaders/GridShader.frag \
    shaders/GridShader.vert \
    shaders/LineShader.frag \
    shaders/LineShader.vert \
    shaders/SphereShader.frag \
    shaders/SphereShader.vert
