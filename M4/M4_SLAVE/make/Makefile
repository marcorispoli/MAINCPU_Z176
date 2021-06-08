#----------------------------------------------------------
# setup required variables 
#----------------------------------------------------------
PROJ_DIR	    = $(abspath /home/user/Desktop/CHIMERA/Workspace/DBT/M4)
MQX_ROOTDIR         = $(PROJ_DIR)/MQX
SHARED_DIR	    = $(PROJ_DIR)/../shared_a5_m4
COM_DIR	    	    = $(PROJ_DIR)/COMMON
SRC_DIR	    	    = $(PROJ_DIR)/M4_SLAVE/source

LIBRARY_ROOTDIR     = $(MQX_ROOTDIR)/lib/$(BOARD).$(TOOL)
APPLICATION_DIR     = ./$(TOOL)/$(LOADNAME)_$(CONFIG)
APPLICATION_FILE    = $(APPLICATION_DIR)/$(NAME).$(APPLICATION_EXT)
LINKER_FILE         = $(call GET_BSP_LINKER_FILE)

TYPE                = application
NAME                = m4_slave
BOARD               = twrvf65gs10_m4


#-----------------------------------------------------------
# load tool settings
#-----------------------------------------------------------
-include tools/$(TOOL).mak


#-----------------------------------------------------------
# search paths
#-----------------------------------------------------------
INCLUDE += $(SRC_DIR)
INCLUDE += $(SRC_DIR)/drivers/nanotec
INCLUDE += $(SRC_DIR)/drivers/lenze
INCLUDE += $(COM_DIR)
INCLUDE += $(SHARED_DIR)
INCLUDE += $(LIBRARY_ROOTDIR)/$(CONFIG)/mcc


SOURCES += $(SRC_DIR)/fault.c
SOURCES += $(SRC_DIR)/slave_can.c 
SOURCES += $(SRC_DIR)/ser240.c
SOURCES += $(SRC_DIR)/actuators.c
SOURCES += $(SRC_DIR)/trx.c
SOURCES += $(SRC_DIR)/arm.c
SOURCES += $(SRC_DIR)/lenze.c
SOURCES += $(SRC_DIR)/main.c
SOURCES += $(SRC_DIR)/drivers/nanotec/pd4.c
SOURCES += $(SRC_DIR)/drivers/lenze/i510.c
SOURCES += $(COM_DIR)/common.c
SOURCES += $(COM_DIR)/canopen.c
SOURCES += $(COM_DIR)/mccClass.c


#----------------------------------------------------------
# include partial makefiles
#----------------------------------------------------------
include $(MQX_ROOTDIR)/build/common/make/global.mak
-include $(MQX_ROOTDIR)/build/$(BOARD)/make/tools/$(TOOL).mak
include $(MQX_ROOTDIR)/build/common/make/verify.mak
include $(MQX_ROOTDIR)/build/common/make/app-process.mak
#----------------------------------------------------------

#----------------------------------------------------------
# generazione ELF file
elf:
	/opt/arm-2017/bin/arm-none-eabi-objcopy -O binary $(APPLICATION_FILE) $(PROJ_DIR)/$(NAME).bin
	cp $(PROJ_DIR)/$(NAME).bin /nfs_chimera/TerminalSlave
	cp $(PROJ_DIR)/$(NAME).bin /nfs_chimera/TerminalMaster
	/opt/arm-2012/bin/arm-none-eabi-readelf $(APPLICATION_FILE) -h | grep "Entry point"


