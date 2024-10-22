#!/bin/bash
# Questo bash fle consente di creare automaticamente tutto 
# il package

# RECUPERA IL CODICE NUMERICO DEL PACKAGE
DIR=${PWD##*/}
ID=${DIR##id}

#################################################################
# DEFINIZIONE DEI CODICI DI REVISIONE
#################################################################
FW269=FW269_1.2.hex
FW190A=FW190A_1.3.hex
FW240=FW240DMD_1.2.hex
FW249U1=FW249U1_3.1.hex
FW249U1A=FW249U1_2.5.hex
FW249U2=FW249U2_2.1.hex
FW249U2A=FW249U2_1.4.hex
FW244A=FW244A_1.1.hex
MAIN=MAINCPU_ANL_1.6
PACKAGE=PACKAGE_ID07.tar
INSTMASTER=manual_install_master.sh
INSTSLAVE=manual_install_slave.sh


#################################################################
# DEFINIZIONE STRUTTURA
#################################################################
HOME=../../RELEASE
FW=$HOME/FW_DBT
MAINCPU=$HOME/$MAIN
DOCDIR=$HOME/DOC
MANUAL=$HOME/MANUAL_INSTALLATION
MANMASTER=$MANUAL/MASTER
MANSLAVE=$MANUAL/SLAVE


#################################################################
# CRERAZIONE PACCHETTO DI RILASCIO NEL FORMATO DEFINITO
#################################################################
rm -r $HOME
mkdir $HOME
mkdir $FW
mkdir $MAINCPU
mkdir $DOCDIR
mkdir $MANUAL
mkdir $MANMASTER
mkdir $MANSLAVE


cp FWDIR/FW269.hex $FW/$FW269
cp FWDIR/FW190A.hex $FW/$FW190A
cp FWDIR/FW240.hex $FW/$FW240
cp FWDIR/FW249U1.hex $FW/$FW249U1
cp FWDIR/FW249U1A.hex $FW/$FW249U1A
cp FWDIR/FW249U2.hex $FW/$FW249U2
cp FWDIR/FW249U2A.hex $FW/$FW249U2A
cp FWDIR/FW244A.hex $FW/$FW244A

cp SWDIR/BIN/* $MAINCPU/
cp SWDIR/ROOTFS/* $MAINCPU/

cp $PACKAGE $MANMASTER/
cp $PACKAGE $MANSLAVE/
cp SWDIR/BATCH/$INSTMASTER $MANMASTER/
cp SWDIR/BATCH/$INSTSLAVE $MANSLAVE/
cp $PACKAGE $HOME/

