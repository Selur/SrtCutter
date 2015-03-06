CONFIG += console
TEMPLATE = app
TARGET = SubtitleCutter
QT += core
HEADERS += cutters/AssCutter.h \
    cutters/Cutter.h \
    cutters/SrtCutter.h
SOURCES += cutters/AssCutter.cpp \
    cutters/Cutter.cpp \
    cutters/SrtCutter.cpp \
    main.cpp
FORMS += 
RESOURCES += 