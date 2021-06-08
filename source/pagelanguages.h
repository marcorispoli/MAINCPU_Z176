#ifndef PAGELANGUAGES_H
#define PAGELANGUAGES_H

#include "application.h"

class PageLanguages : public GWindow
{
    Q_OBJECT

public:
    enum LANGUAGES
    {
        _LNG_ITA=0,
        _LNG_DEU,
        _LNG_FRA,
        _LNG_ENG,
        _LNG_PRT,
        _LNG_RUS,
        _LNG_ESP,
        _LNG_TUR,
        _LNG_POL,
        _LNG_CHN,
        _LNG_LTU
    } ;



    PageLanguages(QApplication* app,  LANGUAGES deflang, bool ls,QString bg,QString bgs ,bool showLogo,int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg);
    virtual ~PageLanguages();
    void childStatusPage(bool stat,int param); // Override funzione della classe base GWindow
                                                // Al cambio pagina riporta lo stato di attivazione
    void timerEvent(QTimerEvent* ev); // Override della classe QObject

    void mousePressEvent(QGraphicsSceneMouseEvent* event); // Override funzione della classe base GWindow
    void setLanguage(LANGUAGES lng);
    bool isLanguage(QString lng);


signals:
    void changeLanguageSgn(); // Al cambio della lingua avverte tutte le pagine di aggiornarsi

public slots:
    void buttonActivationNotify(int id,bool status,int opt);

    void valueChanged(int index,int opt);

public:
    bool localStudy;    // Definisce se lo studio è locale o con PC
    QString localStudyBackground;
    QString openStudyBackground;
    int timerId; // Usato per la gestione del timer della data

    // Testo per la DATA DI SISTEMA
    QGraphicsTextItem* dateText;

    // Testo per Intestazione
    GLabel* intestazioneValue;
    void setIntestazione();

    // Tasti selezione
    GPush* pulsanteIta ;
    GPush* pulsanteSpa ;

    GPush* pulsanteGer ;
    GPush* pulsanteEng ;

    GPush* pulsanteTur ;
    GPush* pulsanteRus ;

    GPush* pulsanteFra ;
    GPush* pulsantePor ;

    // Questo flag è utilizzato dalla funzione di startup per attendere
    // l'istallazione della lingua. Questo FLAG non deve essere utilizzato come diagnostica
    bool linguaOK;
private:
    LANGUAGES selectedLanguage;
    QApplication* parent;
    QTranslator   traduttore;

};

#endif // PAGELANGUAGES_H
