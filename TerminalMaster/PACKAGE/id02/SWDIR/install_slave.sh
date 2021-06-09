#!/bin/bash
# NESSUN PARAMETRO: ISTALLA TUTTO IL CONTENUTO 
ROOT=$1

#VERIFICA ESISTENZA DELLA STRUTTURA DATI
if [ -d "$ROOT/resource" ] ; then
        echo
else
        echo "CREAZIONE STRUTTURA DATI"
        mkdir $ROOT/resource
        mkdir $ROOT/resource/config
fi
                                        
#AGGIORNAMENTO FILE DI SISTEMA
cp BIN/DBTController $ROOT/
cp BIN/m4_slave.bin $ROOT/
cp ROOTFS/mcc.ko $ROOT/lib/modules/3.0.15-ts-armv7l/extra/
cp ROOTFS/dbts $ROOT/
chmod 777 $ROOT/DBTController

cp BATCH/monta.sh $ROOT/
chmod 777 $ROOT/monta.sh

cp BATCH/backup.sh $ROOT/
chmod 777 $ROOT/backup.sh


cp BATCH/restore.sh $ROOT/
chmod 777 $ROOT/restore.sh

sync
