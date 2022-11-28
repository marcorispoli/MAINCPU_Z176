##############################################################
# Definizione del Package di riferimento
##############################################################
PKGCODE=ID04

# Definizione delle directory
HOME=/home/user
CONFIG=/resource/config
INSTALL=/INSTALL


##############################################################
# Sezione standard per tutte le istallazioni manuali e hotfix:
# Viene inserito il nuovo package e copiato il file 
# firmwares.cnf nella directory config. 
# Se file sono cambiati allora il sistema automaticamente
# istallerà tutto al riavvio.
##############################################################
# Pulitura home directory
echo CLEANING UP HOME DIRECTORY ...
mv $HOME/PACKAGE_$PKGCODE.tar /
rm -f $HOME/*
rm -r $HOME/*
mv /PACKAGE_$PKGCODE.tar $HOME/

# Estrazione contenuto in home/user  
echo PACKAGE EXTRACTING ..
cd $HOME
tar -xf PACKAGE_$PKGCODE.tar firmwares.cnf
cp firmwares.cnf $CONFIG/

##############################################################
# Questa sezione è dedicata all'hotfix che non preveda
# una modifica ai files sotto controllo di revisione
##############################################################
rm /resources/config/tomoCfg*.cnf
rm /resources/config/*.bak
rm /resources/config/biopsy.cnf
mv /resources/config/compressore.cnf /resources/config/compressore.id02


##############################################################
# Pulitura home directory
echo CLEANING UP HOME DIRECTORY ...
mv $HOME/PACKAGE_$PKGCODE.tar /
rm -f $HOME/*
rm -r $HOME/*
mv /PACKAGE_$PKGCODE.tar $HOME/

sync

echo PACKAGE "PACKAGE_"$PKGCODE INSTALLED. 










