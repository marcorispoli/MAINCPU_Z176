#!/bin/bash
# Questo bash fle consente di verificare se tutti i firmware sono presenti 

INPUT=$1
DIR=`dirname $0`/  

# LISTA FILES 
FILES="
CONFIG/compressore.cnf 
CONFIG/collimazione_analogica.cnf
CONFIG/armCfg.cnf
CONFIG/lenzeCfg.cnf 
CONFIG/trxCfg.cnf
TUBES/TEMPLATE_XM12L40
TUBES/TEMPLATE_XM12L35
TUBES/TEMPLATE_XM1016L40
TUBES/TEMPLATE_XM1016L35
TUBES/TEMPLATE_XM12H40
TUBES/TEMPLATE_XM12H35
TUBES/TEMPLATE_XM1016H40
TUBES/TEMPLATE_XM1016H35
TUBES/TEMPLATE_XM1016TL40
TUBES/TEMPLATE_XM1016TL35
TUBES/TEMPLATE_XM1016TH40
TUBES/TEMPLATE_XM1016TH35
BIN/DBTController 
BIN/m4_master.bin 
BIN/m4_slave.bin 
ROOTFS/mcc.ko 
ROOTFS/dbtm
ROOTFS/dbts
CONFIG/firmwares.cnf
BATCH/backup.sh
BATCH/restore.sh
BATCH/monta.sh
install_master.sh
install_slave.sh"

EXIST_FILE()
{
FILE=$1
if [ !  -s ${FILE} ] ; then 
 	echo MISSING: $FILE
 	exit 0
fi
}


if [ "$INPUT" == "clean" ] ; then
	rm -f ${DIR}SWPackage.tar
	exit 1
fi

if [ "$INPUT" == "test" ] ; then
	for ff in $FILES ; do
		EXIST_FILE ${DIR}$ff	
	done
	exit 1
fi

if [ "$INPUT" == "build" ] ; then
	cd $DIR
	tar -cf SWPackage.tar $FILES 
	cd -
	exit 1
fi

echo MISSING COMMAND: clean, test, build
exit 0

