
QT += network



TRANSLATIONS += languages/traduzione_ita.ts \
                languages/traduzione_spa.ts \
                languages/traduzione_eng.ts \
                languages/traduzione_ger.ts \
                languages/traduzione_rus.ts \
                languages/traduzione_tur.ts \
                languages/traduzione_por.ts \
                languages/traduzione_pol.ts \
                languages/traduzione_chn.ts \
                languages/traduzione_fra.ts \
                languages/traduzione_ltu.ts \



HEADERS += \
    source/tcpipserver.h \
    source/tcpipclient.h \
    source/echodisplay.h \
    source/resource.h \
    lib/gwindow.h \
    source/mainpage.h \
    lib/dbase.h \
    source/pagelanguages.h \
    source/pagealarms.h \
    source/application.h \
    source/console.h \
    source/generatore.h \
    source/m4.h \
    lib/mcccom.h \
    source/sysio.h \
    source/prototoconsole.h \
    source/protoconsole.h \
    source/serverdebug.h \
    source/appinclude.h \
    source/globvar.h \
    lib/insertcalc.h \
    source/Configuration.h \
    source/config.h \
    source/collimatore.h \
    source/compressor.h \
    source/potter.h \
    lib/msgbox.h \
    source/loader.h \
    source/biopsy.h \
    shared_a5_m4/shared.h \
    source/Service/servicepanelmenu.h \
    source/Service/Calib/calibmenu.h \
    source/startuppage.h \
    source/Service/Calib/calibcompressorposition.h \
    lib/numericpad.h \
    source/Service/Calib/calibcompressorforce.h \
    source/ImagePage.h \
    source/projectionPage.h \
    source/Service/Calib/calibzerosetting.h \
    source/Service/Setup/system.h \
    source/Service/Calib/calibfilter.h \
    source/Service/Calib/calibpower.h \
    source/Service/Calib/calibpot.h \
    source/Service/Calib/calibstarter.h \
    source/Service/Tools/toolsmenu.h \
    source/Service/Tools/tiltingtool.h \
    source/Service/Tools/armtool.h \
    source/Service/Tools/lenzetool.h \
    source/ANALOG/pageOpenAnalogic.h \
    source/ANALOG/pannelloComandi.h \
    source/ANALOG/pannelloCompressione.h \
    source/ANALOG/pannelloSpessore.h \
    source/ANALOG/pannelloKv.h \
    source/ANALOG/pannelloProiezioni.h \
    source/ANALOG/analog.h \
    source/ANALOG/pannelloMas.h \
    source/ANALOG/pannelloOpzioni.h \
    shared_a5_m4/mcc.h \
    shared_a5_m4/errors.h \
    shared_a5_m4/defines.h \
    source/ANALOG/Calibration/pageCalibAnalogic.h \
    source/ANALOG/Calibration/analog_calib.h \
    source/Service/Tools/invertertool.h \
    source/print.h \
    source/audio.h \
    source/AEC.h \
    source/DOSE.h \
    source/Service/Tools/audioTool.h \
    source/systemlog.h \
    source/ANALOG/pannelloColli.h \
    source/Service/Tools/pottertool.h \
    source/ANALOG/pannelloBiopsia.h \
    source/Service/Calib/calibconsole.h \
    source/Service/Calib/calibParking.h \
    source/ANALOG/pannelloMag.h

SOURCES += \
    source/main.cpp \
    source/tcpipserver.cpp \
    source/tcpipclient.cpp \
    source/echodisplay.cpp \
    source/resource.cpp \
    lib/gwindow.cpp \
    source/mainpage.cpp \
    lib/dbase.cpp \
    source/pagelanguages.cpp \
    source/pagealarms.cpp \
    source/console.cpp \
    source/generatore.cpp \
    lib/mcccom.cpp \
    source/sysio.cpp \
    source/prototoconsole.cpp \
    source/protoconsole.cpp \
    source/serverdebug.cpp \
    lib/insertcalc.cpp \
    source/config.cpp \
    source/collimatore.cpp \
    source/compressor.cpp \
    source/potter.cpp \
    lib/msgbox.cpp \
    source/loader.cpp \
    source/biopsy.cpp \
    source/Service/servicepanelmenu.cpp \
    source/Service/Calib/calibmenu.cpp \
    source/startuppage.cpp \
    source/Service/Calib/calibcompressorposition.cpp \
    lib/numericpad.cpp \
    source/Service/Calib/calibcompressorforce.cpp \
    source/ImagePage.cpp \
    source/projectionPage.cpp \
    source/Service/Calib/calibzerosetting.cpp \
    source/Service/Setup/system.cpp \
    source/Service/Calib/calibfilter.cpp \
    source/Service/Calib/calibpower.cpp \
    source/Service/Calib/calibpot.cpp \
    source/Service/Calib/calibstarter.cpp \
    source/Service/Tools/toolsmenu.cpp \
    source/Service/Tools/tiltingtool.cpp \
    source/Service/Tools/armtool.cpp \
    source/Service/Tools/lenzetool.cpp \
    source/ANALOG/pageOpenAnalogic.cpp \
    source/ANALOG/pannelloComandi.cpp \
    source/ANALOG/pannelloCompressione.cpp \
    source/ANALOG/pannelloSpessore.cpp \
    source/ANALOG/pannelloKv.cpp \
    source/ANALOG/pannelloProiezioni.cpp \
    source/ANALOG/pannelloMas.cpp \
    source/ANALOG/pannelloOpzioni.cpp \
    source/ANALOG/analogXrayProcedures.cpp \
    source/ANALOG/Calibration/pageCalibAnalogic.cpp \
    source/ANALOG/Calibration/pageAnalogicDetectorCalibration.cpp \
    source/Service/Tools/invertertool.cpp \
    source/ANALOG/Calibration/pageAnalogicProfileCalibration.cpp \
    source/print.cpp \
    source/ANALOG/Calibration/pageAnalogicTubeCalibration.cpp \
    source/ANALOG/Calibration/pageAnalogicManualExposure.cpp \
    source/audio.cpp \
    source/AEC.cpp \
    source/DOSE.cpp \
    source/Service/Tools/audioTool.cpp \
    source/systemlog.cpp \
    source/ANALOG/Calibration/pageAnalogicCollimationCalibration.cpp \
    source/ANALOG/pannelloColli.cpp \
    source/Service/Tools/pottertool.cpp \
    source/ANALOG/pannelloBiopsia.cpp \
    source/Service/Calib/calibconsole.cpp \
    source/Service/Calib/calibParking.cpp \
    source/ANALOG/pannelloMag.cpp


FORMS += \
    lib/insertcalc.ui \
    lib/msgbox.ui \
    source/Service/servicepanelmenu.ui \
    source/Service/Calib/calibmenu.ui \
    source/startuppage.ui \
    source/Service/Calib/calibcompressorposition.ui \
    lib/numericpad.ui \
    source/Service/Calib/calibcompressorforce.ui \
    source/Service/Calib/calibzerosetting.ui \
    source/Service/Setup/system.ui \
    source/Service/Calib/calibFilter.ui \
    source/Service/Calib/calibPower.ui \
    source/Service/Calib/calibpot.ui \
    source/Service/Calib/calibStarter.ui \
    source/Service/Tools/toolsmenu.ui \
    source/Service/Tools/tilting.ui \
    source/Service/Tools/arm.ui \
    source/Service/Tools/lenze.ui \
    source/ANALOG/analog.ui \
    source/ANALOG/Calibration/analog_calib.ui \
    source/Service/Tools/inverter.ui \
    source/Service/Tools/audio.ui \
    source/Service/Tools/potter.ui \
    source/Service/Tools/toolCalibTube.ui \
    source/Service/Calib/calibconsole.ui \
    source/Service/Calib/calibParking.ui



unix:!macx: LIBS += -L$$/home/user/Desktop/CHIMERA/ltib/rootfs/usr/lib/ -lmcc

INCLUDEPATH += /home/user/Desktop/CHIMERA/ltib/rootfs/usr/include
DEPENDPATH += /home/user/Desktop/CHIMERA/ltib/rootfs/usr/include

RESOURCES += \
    resource/ui/grafica.qrc

OTHER_FILES += \
    source/release.txt

