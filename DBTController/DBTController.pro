#-------------------------------------------------
#
# Project created by QtCreator 2014-08-27T10:07:08
#
#-------------------------------------------------

QT += network
TRANSLATIONS += languages/traduzione_ita.ts \
                languages/traduzione_spa.ts \
                languages/traduzione_eng.ts \
                languages/traduzione_ger.ts \
                languages/traduzione_rus.ts \
                languages/traduzione_tur.ts \
                languages/traduzione_por.ts

HEADERS += \
    source/ProtocolloMET.h \
    lib/QSerialPort.hpp \
    lib/QSerialPort_Global.hpp \
    source/tcpipserver.h \
    source/tcpipclient.h \
    source/echodisplay.h \
    source/resource.h \
    lib/gwindow.h \
    source/mainpage.h \
    lib/dbase.h \
    source/pagelanguages.h \
    source/pageacr.h \
    source/pagealarms.h

SOURCES += \
    source/main.cpp \
    source/ProtocolloMET.cpp \
    lib/QSerialPort.cpp \
    source/tcpipserver.cpp \
    source/tcpipclient.cpp \
    source/echodisplay.cpp \
    source/resource.cpp \
    lib/gwindow.cpp \
    source/mainpage.cpp \
    lib/dbase.cpp \
    source/pagelanguages.cpp \
    source/pageacr.cpp \
    source/pagealarms.cpp


