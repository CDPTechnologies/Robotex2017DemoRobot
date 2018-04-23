TYPE = application
PROJECTNAME = MoverDemoApp

DEPS += \
    gpiopinio \
    i2cio \
    moverdemolib

HEADERS += Libraries.h
SOURCES += CDPMain.cpp

DISTFILES += \
    $$files(*.xml, true) \
    $$files(*.lic, true) \
    $$files(Application/www/*.*, true) \
    Application/www/vue.app-card.js \
    Application/www/semantic.min.js \
    Application/www/protobuf.min.js \
    Application/www/model.js \
    Application/www/vue.component-card.js \
    Application/www/bytebuffer.min.js \
    Application/www/vue.expand.js \
    Application/www/vue.common.min.js \
    Application/www/jquery.slim.min.js \
    Application/www/long.min.js \
    Application/www/vue.min.js \
    Application/www/themes/default/assets/fonts/icons.woff \
    Application/www/themes/default/assets/fonts/icons.woff2 \
    Application/www/themes/basic/assets/fonts/icons.ttf \
    Application/www/themes/default/assets/fonts/icons.ttf \
    Application/www/images/cdpstudio.png \
    Application/www/themes/basic/assets/fonts/icons.svg \
    Application/www/themes/default/assets/fonts/icons.svg \
    Application/www/semantic.min.css \
    Application/www/vue.common.min.js.map \
    Application/www/jquery.slim.min.map

load(cdp)

ID = 168039552929066 //do not change
