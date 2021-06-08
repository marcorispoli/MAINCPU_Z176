#ifndef GWINDOW_H
#define GWINDOW_H

#include <QtGui>
#include <QApplication>
#include <QGraphicsView>
#include <cstdarg>          // Per la lista di argomenti variabili



// Classe di riferimento per GWindow per la segnalazione interna
class GWinObj : public QObject
{
    Q_OBJECT

public:
    GWinObj();



    void setNewPage(int pg, int prevPg, int param); // Richiesta di cambio pagina dalle classi


    int  parentPage;        // Pagina attiva al momento del cambio pagina
    int  curPage;           // Pagina correntemente selezionata
    int  maxPage;           // Massimo numero di pagina presente
    bool curPageVisible;    // Indica se la pagina corrente deve essere visulaizzata o no

    void pushActivate(int id, bool stat,int opt)
    {
        emit pushActivationSgn(id,stat,opt);
    }

    // Richiesto da funzione di attivazione pulsante per resettare eventuali combo lincati ad esso
    void comboResetReq(int id, int index)
    {
        emit pushComboActivationSgn(id,index);
    }

    QList<QObject*> pushList;   // Lista puntatori bottoni creati

    bool changePagePending;     // Cambio pagina in corso. Nessun cambio pagina può avvenire

signals:
    void setNewPageEcho(int pg, int parentPage, int opt);
    void changePage(int Page,int opt);  // Segnale per sincronizzare le pagine
    void pushComboActivationSgn(int id, int index); // Segnale per sincronizzare i pulsanti in combo (index!=0)
    void pushActivationSgn(int id, bool status,int opt); // Segnale di attivazione singolo pulsante


};

#ifndef _GWINDOW_CPP
extern GWinObj GWindowRoot; // Dichiara l'oggetto di riferimento per la classe
#endif

/******************************************************************
  Classe contenitore di oggetti graphicsGUI e derivati

  CARATTERISTICHE:

  - Deve avere uno sfondo su cui agiscono gli item grafici;
  - Deve gestire lo scorrimento delle pagine tramite due zone: forward/backward;

*/
class GWindow: public QGraphicsScene
{
    Q_OBJECT

public:
    GWindow(QString bg, bool showLogo, int w,int h, qreal angolo,QPainterPath pn, int pgpn, QPainterPath pp, int pgpp, int pg);

    // Sfondo e vista
    QPixmap background;             // Immagine di sfondo
    void setBackground(QString bg); // Imposta un nuovo background


    QGraphicsView* view; // Puntatore alla View corrente
    int view_width;      // Larghezza vista corrente
    int view_height;     // Altezza vista corrente
    qreal view_angle;    // Angolo vista corrente

    // Gestione dello scorrimento pagine
    QPainterPath    nextPagePath;
    QPainterPath    prevPagePath;
    int             nextPage;
    int             prevPage;
    bool            nextPageEnabled; // Abilitazione pagina successiva
    bool            prevPageEnabled; // Abilitazione pagina successiva

    int             indexPage;           // codice pagina assegnato
    bool            status;              // Stato di attivazione della pagina
    static void     setPage(int index, int prevPg, int param);  // Richiesta di cambio pagina di classe
    bool            activatePage(int param);       // Richiesta di attivazione della pagina specifica(se non attiva)
    bool            isCurrentPage();               // Restituisce true se Ã¨ la pagina corrente

    // Metodi della classe
    void rotate(qreal angolo) { view->rotate(angolo);view_angle+=angolo;  } // Permette di ruotare la vista corrente
    static QPainterPath setPointPath(int i, ...);  // Permette di costruire un path per punti

    QPainterPath shape() const
    {
        return PagePath;
    }

public slots:
    void changePage(int pg,  int param); // Segnale di cambio pagina da GWinObj

protected:
    virtual void nextPageHandler(void);
    virtual void prevPageHandler(void);

    // Funzioni base di gestione mouse
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
 //   virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
    virtual void childStatusPage(bool stat, int param) {if(stat) param=0;} // Funzione per le classi derivate chiamata al cambio pagina

private:
    QPainterPath    PagePath;
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Classe per la gestione di pulsanti ON/OFF con possibilitÃ  di associazione in combo

class GPushItem: public QGraphicsItem
{

    Q_DECLARE_TR_FUNCTIONS(GPushItem)


public:
    GPushItem(QObject* parent);
     ~GPushItem() {}

    // REttangolo per ottimizzare lka ricerca della pressione del tasto
    QRectF boundingRect() const;

    // Shape utilizzato per il riconoscimento della pressione del tasto
    QPainterPath shape() const;


    // Funzione di disegno chiamata con update() o nei casi di modifica area grafica nel rettangolo
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected:
    // Funzioni base di gestione mouse
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

public:
    QObject*  parent;  // Window di appartenenza
};

class GPush: public QObject
{

    Q_OBJECT

public:
    GPush(GWindow* parent, QPixmap pix, QPainterPath path, int x, int y, int index, int data, bool state=false); // Standard
    GPush(GWindow* parent, QPixmap pix, QPainterPath path, int x, int y, GPush* copyObj); // Pulsante Copia
    GPush(GWindow* parent, QPixmap notActivatedPix, QPixmap activatedPix, QPainterPath path, int x, int y, int index, int data, bool state=false);
    GPush(GWindow* parent, QPainterPath path, int x, int y, int index, int data, bool state); // Versione pulsante trasparente
    GPush(GWindow* parent, QPixmap notActivatedPix, QPixmap activatedPix, QPixmap disabledPix, QPainterPath path, int x, int y, int index, int data, bool state, bool visible, bool enabled);
    GPush(GWindow* parent, QPixmap* notActivatedPix, QPixmap* activatedPix, QPixmap* disabledPix, QPainterPath path, int x, int y, int index, int data, bool state, bool visible, bool enabled);



     ~GPush() {}

    void  activate(bool status,int opt=0); // Attivazione stato del pulsante
    static void pushActivate(int id, bool status, int opt=0); // Funzione della classe per attivare un pulsante da id
    static void pushResetCombo(int code, int opt=0); // Funzione della classe disattivare tutti i combo con indice code

    static void pushRefresh(int opt); // Funzione della classe per rinfrscare lo stato di tutti i pulsanti
    void setEnable(bool ena);         // Attiva la configurazione del pulsante Enable/Disable
    void setVisible(bool ena);        // Abilita o disabilita il pulsante

    void setPix(QString nap, QString ap); // Imposta le immagini relative alle selezioni
    void setPix(QString ap);              // Imposta solo immagine ON
    void setOptions(int options) {this->opt=options;}

private slots:
    void pushComboNotify(int id, int index); // Ricezione stato di attivazione da pulsante tramite GWindow
    void pushActivationNotify(int id, bool status,int opt); // Segnale di attivazione singolo pulsante

public:
    static  int             classId;    // contatore degli Id usati
    int                  pulsanteId;    // Id personale del pulsante creato e derivato da quello della classe
    GPush*               pulsanteCopia;
    GPushItem*           pulsanteItem;  // puntatore all'item grafico
    GWindow*             parentWindow;  // Window di appartenenza
    QPixmap*             pulsanteActivatedPix;   // Pixmap in caso di pressione
    QPixmap*             pulsanteNotActivatedPix;   // Pixmap in caso di pressione
    QPixmap*             pulsanteDisabledPix;   // Pixmap in caso di pulsante disabilitato

    QPainterPath*        pulsantePath;  // pathe di riconoscimento della pressione del pulsante
    QRectF               pulsanteBound; // Riquadro in coordinate scena
    bool                 pulsanteState; // Stato di attivazione del pulsante
    bool                 pulsanteEnabled; // Stati disable/Enable
    int                  pulsanteIndex; // Indice di associazione a COMBO se diverso da zero
    int                  posX;          // Posizione angolo top-left
    int                  posY;
    int                  pulsanteData; // Dato

    int                  opt; // Imposta le opzioni per le modalità echo
};

class GLabel: public QGraphicsItem
{

    Q_DECLARE_TR_FUNCTIONS(GLabel)

public:
    GLabel(GWindow* parent, QRectF bound, QFont font, QColor color,char* testo="", Qt::AlignmentFlag flags=Qt::AlignCenter);
    GLabel(GWindow* parent, QRectF bound, QFont font, QColor color,QString testo="", Qt::AlignmentFlag flags=Qt::AlignCenter);
    GLabel(QGraphicsScene* parent, QRectF bound, QFont font, QColor color,QString testo="", Qt::AlignmentFlag flags=Qt::AlignCenter);

    ~GLabel() {}

    void setPlainText(char* testo);
    void setVisible(bool stat);


   // Determina il Bound Dell'Item
   QRectF boundingRect() const
   {
       return labelBound;
   }

   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

public:
    GWindow*             parentWindow;  // Window di appartenenza
    QRectF               labelBound;    // Riquadro in coordinate scena
    QColor               labelColor;    // Stato di attivazione del pulsante
    QFont                labelFont;     // Font della label
    QString              labelText;     // Contenuto testuale formattato
    Qt::AlignmentFlag    alignmentText; // Flag per l'allinemaento rispetto a labelBound
};






#endif // GWINDOW_H
