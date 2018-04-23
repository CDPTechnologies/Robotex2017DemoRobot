CDPVERSION = 4.5
TYPE = library
PROJECTNAME = MoverDemoLib

DEPS += \

HEADERS += \
    moverdemolib.h \
    MoverDemoLibBuilder.h \
    Controller.h

SOURCES += \
    MoverDemoLibBuilder.cpp \
    Controller.cpp

DISTFILES += $$files(*.xml, true) \
    Templates/Models/MoverDemoLib.Controller.xml

load(cdp)
