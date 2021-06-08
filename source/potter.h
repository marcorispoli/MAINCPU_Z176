#ifndef POTTER_H
#define POTTER_H

#include "application.h"

class Potter: public QObject
{
    Q_OBJECT

public:
    explicit Potter(QObject *parent = 0);
    void activateConnections(void);

    unsigned char getPotId(){return potterId;}
    unsigned char getPotFactor(){return potterFactor;}

    bool isValid() {return (potterId!=POTTER_UNDEFINED);}
    bool isMagnifier() {return (potterId==POTTER_MAGNIFIER);}
    bool isValidMagFactor() {return potterValidFactor;}

    bool setDetectorField(unsigned char val);
    bool getCassettePresence(){return cassette;}
    bool getCassetteExposed(){return cassetteExposed;}

    void startTestGrid(int nTest); // Attivazione test griglia

public slots:
    void guiNotify(unsigned char id, unsigned char mcccode, QByteArray data); // Notifiche da GUI

private:
    unsigned char potterId;      // indica il codice identificativo dell'accessorio
    unsigned char potterFactor;  // indica l'ultimo aggiornamento del fattore di ingrandimento (255 se non definito)
    bool          potterValidFactor;   // Indica se il dato del potter Ã¨ valido
    bool          cassette;     // Presenza cassetta (only analogic)
    bool          cassetteExposed;     // cassetta già esposta (only analogic)

};

#endif // POTTER_H
