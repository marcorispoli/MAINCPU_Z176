##############################################################
# Definizione del Package di riferimento
##############################################################
PKGCODE=ID06

# Definizione delle directory
HOME=/home/user
CONFIG=/resource/config
TUBES=/resource/config/Tubes
INSTALL=/INSTALL


##############################################################
# Sezione standard per tutte le istallazioni manuali e hotfix:
# Viene inserito il nuovo package e copiato il file 
# firmwares.cnf nella directory config. 
# Se file sono cambiati allora il sistema automaticamente
# istallerà tutto al riavvio.
##############################################################

# Estrazione contenuto in home/user  
echo PACKAGE EXTRACTING ..
cd $HOME
tar -xf PACKAGE_$PKGCODE.tar 
tar -xf SWPackage.tar 

cp ./CONFIG/firmwares.cnf $CONFIG/
cp ./BIN/DBTController /
cp ./BIN/m4_master.bin /
cp ./ROOTFS/dbtm /
cp -r ./TUBES/* $TUBES/

sync
chmod 777 /DBTController
chmod 777 /dbtm


##############################################################
# Questa sezione è dedicata all'hotfix che non preveda
# una modifica ai files sotto controllo di revisione
##############################################################


##############################################################
# Pulitura home directory
echo CLEANING UP HOME DIRECTORY ...
cd $HOME
rm ./SWPackage.tar
rm ./FirmwarePackage.tar
rm -r ./BATCH
rm -r ./BIN
rm -r ./CONFIG
rm -r ./ROOTFS
rm -r ./TUBES

sync

echo PACKAGE "PACKAGE_"$PKGCODE INSTALLED. 










