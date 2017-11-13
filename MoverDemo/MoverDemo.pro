CDPVERSION = 4.3
TYPE = system
load(cdp)

DISTFILES += $$files(*.xml, false)

SUBDIRS += \
    MoverDemoApp
