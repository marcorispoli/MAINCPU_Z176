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
	echo ERRORE! MANCA IL NOME DEL FILE DI RESTORE 
	exit
fi
echo "RESTORE DEL FILE" $HOME/$1.tar " IN CORSO ...
mv $HOME/$1.tar /

cd /
tar -xf $1.tar  
rm $1.tar 

sync






