TEMPLATE = app
DESTDIR  = bin/
HEADERS += src/*.h
SOURCES += src/*.cpp lib/kit/kit/log/*.cpp lib/kit/kit/args/*.cpp
INCLUDEPATH += lib/kit/ lib/kit/lib/local_shared_ptr
LIBS += -L/usr/local/lib \
    -lpthread \
    -lboost_thread \
    -lboost_system \
    -lboost_regex \
    -lboost_filesystem \
    -ljsoncpp \

FORMS += src/ui/*.ui
UI_DIR += src/ui/
MOC_DIR = moc
DEFINES += QT_DLL META_STORAGE=std::shared_ptr META_THIS=std::enable_shared_from_this
CONFIG  += qt warn_on debug c++11 no_keywords
QT += core gui widgets svg
OBJECTS_DIR = obj/
QMAKE_CXXFLAGS += -std=c++11
