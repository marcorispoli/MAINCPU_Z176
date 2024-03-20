#!/bin/bash
# NESSUN PARAMETRO: ISTALLA TUTTO IL CONTENUTO 
ROOT=$1


#Copia solo se il file non esiste
# PARAM: FILE, DIR-ORIGIN, DIR-DEST
COPY_IF_NOT_EXIST()
{
FILE=$1
ORIGINE=$2
DESTINAZIONE=$3

if [ -s ${DESTINAZIONE}${FILE} ] ; then
	#SE IL FILE ESISTE
	echo "FILE $FILE GIA ESISTENTE"
else
	#SE IL FILE  NON ESISTE
	echo "COPY FILE $FILE TO $DESTINAZIONE"
	cp ${ORIGINE}${FILE} ${DESTINAZIONE}${FILE}
fi
}

#VERIFICA ESISTENZA DELLA STRUTTURA DATI
if [ -d "$ROOT/resource" ] ; then
	echo
else
	echo "CREAZIONE STRUTTURA DATI"
	mkdir $ROOT/resource
	mkdir $ROOT/resource/config
	mkdir $ROOT/resource/config/Tubes
fi

#FILES DI CONFIGURAZIONE
COPY_IF_NOT_EXIST compressore.cnf ./CONFIG/ $ROOT/resource/config/
COPY_IF_NOT_EXIST collimazione_analogica.cnf ./CONFIG/ $ROOT/resource/config/
sync

cp CONFIG/armCfg.cnf $ROOT/resource/config/
cp CONFIG/trxCfg.cnf $ROOT/resource/config/
cp CONFIG/lenzeCfg.cnf $ROOT/resource/config/
cp CONFIG/firmwares.cnf $ROOT/resource/config/
sync

#AGGIORNAMENTO TEMPLATES TUBI
rm -r $ROOT/resource/config/Tubes/TEMPLATE_*
cp -r TUBES/TEMPLATE_* $ROOT/resource/config/Tubes/
sync

#AGGIORNAMENTO FILE DI SISTEMA
cp BIN/DBTController $ROOT/
cp BIN/m4_master.bin $ROOT/
cp ROOTFS/mcc.ko $ROOT/lib/modules/3.0.15-ts-armv7l/extra/
cp ROOTFS/dbtm $ROOT/
sync

chmod 777 $ROOT/DBTController

cp BATCH/backup.sh $ROOT/
cp BATCH/restore.sh $ROOT/
cp BATCH/monta.sh $ROOT/
chmod 777 $ROOT/backup.sh
chmod 777 $ROOT/restore.sh
chmod 777 $ROOT/monta.sh

cp BATCH/manual_install_master.sh /home/user
chmod 777 /home/user/manual_install_master.sh
chmod 777 $ROOT/dbtm

sync


