#!/bin/bash
##############################################################
# Questo batch esegue il backup del sistema
# Il batch richiede un parametro come nome
# Il risultato del backup viene sempre salvato in $HOME
##############################################################
# Definizione delle directory
HOME=/home/user
CONFIG=/resource/config

# controllo sul numero dei parametri
if [ "$#" -ne 1 ]; then
	echo ERRORE! MANCA IL NOME DEL FILE DI BACKUP
	exit
fi
 
echo "BACKUP DEL SISTEMA IN CORSO"
echo "IL FILE BACKUP E':" $HOME/$1.tar

# Il backup viene fatto dalla root
cd /
tar -cf $1.tar resource  $HOME DBTController m4_master.bin 
mv $1.tar $HOME

sync






