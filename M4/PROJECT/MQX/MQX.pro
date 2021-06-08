TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ../../MQX/mqx/source/io/can/flexcan/fsl_flexcan_int.c \
    ../../MQX/mqx/source/io/can/flexcan/fsl_flexcan_hal.c \
    ../../MQX/mqx/source/io/can/flexcan/fsl_flexcan_driver.c

HEADERS += \
    ../../MQX/mqx/source/io/can/flexcan/fsl_flexcan_int.h \
    ../../MQX/mqx/source/io/can/flexcan/fsl_flexcan_hal.h \
    ../../MQX/mqx/source/io/can/flexcan/fsl_flexcan_driver.h \
    ../../MQX/mqx/source/bsp/twrvf65gs10_m4/twrvf65gs10_m4.h \
    ../../MQX/mqx/source/bsp/twrvf65gs10_m4/bsp.h \
    ../../MQX/config/twrvf65gs10_m4/user_config.h \
    ../../MQX/mqx/source/io/lwgpio/lwgpio_vgpio.h \
    ../../MQX/mqx/source/include/mqx.h

