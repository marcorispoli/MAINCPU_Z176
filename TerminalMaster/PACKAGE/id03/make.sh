#!/bin/bash
# Questo bash fle consente di creare automaticamente tutto 
# il package

# RECUPERA IL CODICE NUMERICO DEL PACKAGE
DIR=${PWD##*/}
ID=${DIR##id}

INPUT=$1
if [ "$INPUT" == "?" ] ; then
	echo "make.sh xx: compila tutto il package e produce PACKAGE_IDxx.tar"
	echo "make.sh clean: cancella tutti i files temporanei"
	echo "make.sh test: effettua il test di completezza contenuto"
	
	exit 0
fi


if [ "$INPUT" == "clean" ] ; then
	echo PACKAGE CLEANING ..
	FWDIR/make.sh clean
	SWDIR/make.sh clean	
	rm PACKAGE_ID*.tar
	exit 1
fi

if [ "$INPUT" == "test" ] ; then
	echo PACKAGE  TESTING ..
	FWDIR/make.sh test
	SWDIR/make.sh test
	exit 1
fi


#####################################################
# Verifica preliminare che tutto sia in ordine
FWDIR/make.sh test
if [ $? == 0 ] ; then
	echo Archivio Firmwares non corretto
	exit 0
fi 

SWDIR/make.sh test
if [ $? == 0 ] ; then
	echo Archivio Software non corretto 
	exit 0
fi 

####################################################

#Nome del file archivio 
PACKAGE_FILE="PACKAGE_ID${ID}.tar"
echo CREAZIONE DEL PACKAGE: $PACKAGE_FILE


# cancella gli archivi già realizzati
rm -f *.tar

FWDIR/make.sh clean
SWDIR/make.sh clean

# Creazione del FirmwarePackage.tar
FWDIR/make.sh build 
SWDIR/make.sh build 

#Creazione del PACKAGE FILE
mv FWDIR/FirmwarePackage.tar ./
mv SWDIR/SWPackage.tar ./
cp SWDIR/CONFIG/firmwares.cnf ./
tar -cf $PACKAGE_FILE FirmwarePackage.tar firmwares.cnf SWPackage.tar
cp $PACKAGE_FILE ../
rm -f FirmwarePackage.tar
rm -f SWPackage.tar
rm -f firmwares.cnf

#################################################################

