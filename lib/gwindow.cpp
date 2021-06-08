#define _GWINDOW_CPP
#include "gwindow.h"
#include "../source/print.h"




GWinObj GWindowRoot; // Dichiara l'oggetto di riferimento per la classe
int GPush::classId=0;      // Inizializza il contatore degli Id sulla classe base

GWinObj::GWinObj()
{
    curPage = 0;
    parentPage = 0;
    maxPage = 0;
    curPageVisible=TRUE;
    pushList.clear(); // Cancella la lista di puntatori ai bottoni
    changePagePending = false;

}

// Attenzione con il parentPage: soloil terminale che usa setNewPage
// avrà il parentPage corretto. L'altro terminale nonlo avrà aggiornato!!!
void GWinObj::setNewPage(int pg, int prevPg, int param)
{

    if(changePagePending) {
        PRINT(QString("<GWinRoot->setNewPage> RICORSIVO. ORIGINE:%1, DESTINAZIONE:%2").arg(curPage).arg(pg));
        return;
    }

    if(pg<0) return;

    changePagePending=TRUE;

    // Se la pagina è già attiva non fa nulla
    if(pg==curPage) {
        changePagePending=FALSE;
        return;
    }

    // Salva il codice della pagina corrente
    parentPage = prevPg;
    PRINT(QString("<GWinRoot->setNewPage> (A): CURPG:%1 PARENT:%2, REQPG:%3").arg(curPage).arg(parentPage).arg(pg));

    // Invia all'ECO l'ordine del cambio pagina
    emit setNewPageEcho(pg,parentPage,param);

    // fa prima uscire tutti quanti: naturalmente solo quella corrente esegue il proprio codice di uscita
    emit changePage(0,param);
    curPage = 0;    

    // ora seleziona la pagina da attivare: ora solo la destinataria può eseguire le sue inizializzazioni    
    emit changePage(pg,param);
    curPage = pg;

    changePagePending=FALSE;
}



GWindow::GWindow(QString bg, bool showLogo, int w,int h, qreal angolo,QPainterPath pn,  int pgpn, QPainterPath pp, int pgpp, int pg)
{
    // SETUP SCENA
    background.load(bg);
    view_width =  w;
    view_height = h;
    view_angle = angolo;
    indexPage = pg;
    nextPage=pgpn;
    prevPage=pgpp;
    nextPageEnabled = TRUE;
    prevPageEnabled = TRUE;

    QGraphicsScene::setSceneRect(0,0,background.width(), background.height());

    // Attiva il background principale
    this->setBackgroundBrush(background);

    // IMPOSTAZIONE VISTA STANDARD
    // Viene impostata la dimensione della vista in funzione delle dimensioni LCD
    view = new QGraphicsView(this);
    view->setWindowFlags(Qt::FramelessWindowHint);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setFixedSize(w,h);    // Dimensione della vista
    view->rotate(angolo);       // Angolo di rotazione della vista corrente


    // Gestione scorrimento pagine
    PagePath = nextPagePath = pn;
    PagePath.addPath(prevPagePath=pp);
    if(indexPage>GWindowRoot.maxPage)
        GWindowRoot.maxPage = indexPage;
    connect(&GWindowRoot,SIGNAL(changePage(int,int)),this,SLOT(changePage(int,int)),Qt::UniqueConnection);


    // Attivazione pagina corrente: di default Ã¨ la pagina 0
    if((indexPage==0))
    {
        if(GWindowRoot.curPageVisible) view->show();
        else view->hide();
        status = true;
    }
    else
    {
        status = false;
        view->hide();
    }


}

// Richiede un cambio pagina alla classe GWindow.
// <param> Ã¨ utilizzato dall'applicazione per passare un contesto
// che arriverÃ  al destinatario del cambio pagina
void GWindow::setPage(int index,int prevPg,int param)
{
    GWindowRoot.setNewPage(index,prevPg,param);
}

// Attiva la pagina assegnata alla classe oggetto e
// restituisce true se Ã¨ un cambio pagina
bool GWindow::activatePage(int param)
{
    GWindowRoot.setNewPage(this->indexPage,GWindowRoot.curPage, param);
    return false;
}

bool GWindow::isCurrentPage()
{
    return (GWindowRoot.curPage==this->indexPage);
}

void GWindow::setBackground(QString bg)
{
    background.load(bg);
    QGraphicsScene::setSceneRect(0,0,background.width(), background.height());
    this->setBackgroundBrush(background);

}

QPainterPath GWindow::setPointPath(int num, ...)
{
    QPolygonF poly;
    QPainterPath pathloc;
    QPoint point;

    va_list args;
    va_start(args,num);
    while(num>0)
    {
        point.setX(va_arg(args,int));
        point.setY(va_arg(args,int));
        num-=2;
        poly.append(point);
    }

    va_end(args);

    pathloc.addPolygon(poly);
    pathloc.closeSubpath();
    return pathloc;
}

// Segnale ricevuto dalla classe GWinObj per dichiarare chi Ã¨ la pagina attiva
void GWindow::changePage(int pg,int param)
{    

    // Attivazione della pagina
    if(indexPage==pg) status = true;
    else status = false;

    // Visualizzazione della pagina
    if(GWindowRoot.curPageVisible== FALSE) this->view->hide();
    else if(!status) this->view->hide();
    else this->view->show();

    // Notifica del cambio di stato
    if((status) || (isCurrentPage()))
        childStatusPage(status,param);

}

void GWindow::nextPageHandler(void)
{
    GWindowRoot.setNewPage(nextPage,GWindowRoot.curPage,0);
    return;
}
void GWindow::prevPageHandler(void)
{
    GWindowRoot.setNewPage(prevPage,GWindowRoot.curPage,0);
    return;
}

void GWindow::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    // Gestione della pressione del mouse per il cambio pagina
    if(nextPagePath.contains(event->scenePos())){
        if(nextPageEnabled) nextPageHandler();
    }else if(prevPagePath.contains(event->scenePos())){
        if(prevPageEnabled) prevPageHandler();
    }

    QGraphicsScene::mousePressEvent(event);

}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

// FUNZIONE CHIAMATA IN CASO DI PAINT




GPushItem::GPushItem(QObject* p)
{
    parent=p;
}

QRectF GPushItem::boundingRect() const
{
    return ((GPush*)parent)->pulsanteBound; //QRectF(((GPush*)parent)->pulsantePix.rect());
}

// Shape utilizzato per il riconoscimento della pressione del tasto
QPainterPath GPushItem::shape() const
{
    return *((GPush*)parent)->pulsantePath;
}
void GPushItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{

    if (((GPush*)parent)->pulsanteEnabled==false)
        return;

    // In caso di combo almeno uno deve essere attivo!
    //if((((GPush*)parent)->pulsanteIndex)&&(((GPush*)parent)->pulsanteState))
    //    return;

    // Attivazione/Disattivazione pulsante: usa la classe Root per la propria attivazione
    GWindowRoot.pushActivate(((GPush*)parent)->pulsanteId, !((GPush*)parent)->pulsanteState,((GPush*)parent)->opt);

}

void GPushItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    return;
}

void GPushItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Con disabilitato non mostra nulla
    if (((GPush*)parent)->pulsanteEnabled==false){
        if(((GPush*)parent)->pulsanteDisabledPix!=0)  painter->drawPixmap(((GPush*)parent)->posX,((GPush*)parent)->posY,*(((GPush*)parent)->pulsanteDisabledPix));
        return;
    }

    if ((((GPush*)parent)->pulsanteState==false)&&(((GPush*)parent)->pulsanteNotActivatedPix != 0))
    {
        painter->drawPixmap(((GPush*)parent)->posX,((GPush*)parent)->posY,*(((GPush*)parent)->pulsanteNotActivatedPix));
        return;
    }

    // Visualizzazione immagine di selezione
    if ((((GPush*)parent)->pulsanteState==true)&&(((GPush*)parent)->pulsanteActivatedPix != 0))
    {
        painter->drawPixmap(((GPush*)parent)->posX,((GPush*)parent)->posY,*(((GPush*)parent)->pulsanteActivatedPix));
        return;
    }

}



// PULSANTE CON SOLO LA VISUALIZZAZIONE DURANTE ATTIVAZIONE
GPush::GPush(GWindow* parent, QPixmap activatedPix, QPainterPath path, int x, int y, int index, int data, bool state)
{
    pulsanteId = GPush::classId++;
    pulsanteIndex = index;
    parentWindow = parent;

    pulsanteDisabledPix = 0;
    pulsanteNotActivatedPix = 0;
    pulsanteActivatedPix = new QPixmap(activatedPix);
    pulsantePath = new QPainterPath(path);

    pulsanteState=state;
    pulsanteData = data;
    pulsanteEnabled=true;
    posX=x;
    posY=y;
    pulsanteCopia=0;

    // Crea Item grafico associato al pulsante
    pulsanteBound = QRectF(pulsanteActivatedPix->rect());
    pulsanteBound.moveTo(x,y);

    pulsanteItem = new GPushItem((QObject*)this);
    parentWindow->addItem(pulsanteItem);

    // Attiva la connessione per le opzioni combo ma solo se index !=0
    if(index)
    {
        connect(&GWindowRoot,SIGNAL(pushComboActivationSgn(int,int)),this,SLOT(pushComboNotify(int,int)),Qt::UniqueConnection);
    }

    // Connessione al segnale di Root per attivazione esterna
    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)),this,SLOT(pushActivationNotify(int,bool,int)),Qt::UniqueConnection);

    // Registrazione pulsante nella classe GWindowRoot
    GWindowRoot.pushList.append((QObject*)this);
    opt =0;

}

// Pulsante con tripla visualizzazione di stato
GPush::GPush(GWindow* parent, QPixmap notActivatedPix, QPixmap activatedPix, QPixmap disabledPix, QPainterPath path, int x, int y, int index, int data, bool state, bool visible, bool enabled)
{
    pulsanteId = GPush::classId++;
    pulsanteIndex = index;
    parentWindow = parent;

    pulsanteDisabledPix = new QPixmap(disabledPix);
    pulsanteNotActivatedPix = new QPixmap(notActivatedPix);
    pulsanteActivatedPix = new QPixmap(activatedPix);
    pulsantePath = new QPainterPath(path);

    pulsanteState=state;
    pulsanteData = data;
    pulsanteEnabled=enabled;
    posX=x;
    posY=y;
    pulsanteCopia=0;

    // Crea Item grafico associato al pulsante
    pulsanteBound = QRectF(pulsanteActivatedPix->rect());
    pulsanteBound.moveTo(x,y);

    pulsanteItem = new GPushItem((QObject*)this);
    parentWindow->addItem(pulsanteItem);

    // Attiva la connessione per le opzioni combo ma solo se index !=0
    if(index)
    {
        connect(&GWindowRoot,SIGNAL(pushComboActivationSgn(int,int)),this,SLOT(pushComboNotify(int,int)),Qt::UniqueConnection);
    }

    // Connessione al segnale di Root per attivazione esterna
    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)),this,SLOT(pushActivationNotify(int,bool,int)),Qt::UniqueConnection);

    // Registrazione pulsante nella classe GWindowRoot
    GWindowRoot.pushList.append((QObject*)this);

    opt =0;
    setVisible(visible);
}



// Pulsante con tripla visualizzazione di stato e puntatori alle pixmap
GPush::GPush(GWindow* parent, QPixmap* notActivatedPix, QPixmap* activatedPix, QPixmap* disabledPix, QPainterPath path, int x, int y, int index, int data, bool state, bool visible, bool enabled)
{
    pulsanteId = GPush::classId++;
    pulsanteIndex = index;
    parentWindow = parent;

    // Questo pulsante usa delle pixmap fornite dalla pagina chiamante
    pulsanteDisabledPix = disabledPix;
    pulsanteNotActivatedPix = notActivatedPix;
    pulsanteActivatedPix = activatedPix;

    pulsantePath = new QPainterPath(path);

    pulsanteState=state;
    pulsanteData = data;
    pulsanteEnabled=enabled;
    posX=x;
    posY=y;
    pulsanteCopia=0;

    // Crea Item grafico associato al pulsante
    pulsanteBound = QRectF(pulsanteActivatedPix->rect());
    pulsanteBound.moveTo(x,y);

    pulsanteItem = new GPushItem((QObject*)this);
    parentWindow->addItem(pulsanteItem);

    // Attiva la connessione per le opzioni combo ma solo se index !=0
    if(index)
    {
        connect(&GWindowRoot,SIGNAL(pushComboActivationSgn(int,int)),this,SLOT(pushComboNotify(int,int)),Qt::UniqueConnection);
    }

    // Connessione al segnale di Root per attivazione esterna
    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)),this,SLOT(pushActivationNotify(int,bool,int)),Qt::UniqueConnection);

    // Registrazione pulsante nella classe GWindowRoot
    GWindowRoot.pushList.append((QObject*)this);

    opt =0;
    setVisible(visible);
}


// Pulsante con doppia visualizzazione di stato
GPush::GPush(GWindow* parent, QPixmap notActivatedPix, QPixmap activatedPix, QPainterPath path, int x, int y, int index, int data, bool state)
{
    pulsanteId = GPush::classId++;
    pulsanteIndex = index;
    parentWindow = parent;

    pulsanteDisabledPix = 0;
    pulsanteNotActivatedPix = new QPixmap(notActivatedPix);
    pulsanteActivatedPix = new QPixmap(activatedPix);
    pulsantePath = new QPainterPath(path);

    pulsanteState=state;
    pulsanteData = data;
    pulsanteEnabled=true;
    posX=x;
    posY=y;
    pulsanteCopia=0;

    // Crea Item grafico associato al pulsante
    pulsanteBound = QRectF(pulsanteActivatedPix->rect());
    pulsanteBound.moveTo(x,y);

    pulsanteItem = new GPushItem((QObject*)this);
    parentWindow->addItem(pulsanteItem);

    // Attiva la connessione per le opzioni combo ma solo se index !=0
    if(index)
    {
        connect(&GWindowRoot,SIGNAL(pushComboActivationSgn(int,int)),this,SLOT(pushComboNotify(int,int)),Qt::UniqueConnection);
    }

    // Connessione al segnale di Root per attivazione esterna
    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)),this,SLOT(pushActivationNotify(int,bool,int)),Qt::UniqueConnection);

    // Registrazione pulsante nella classe GWindowRoot
    GWindowRoot.pushList.append((QObject*)this);

    opt =0;

}


GPush::GPush(GWindow* parent, QPixmap activatedPix, QPainterPath path, int x, int y, GPush* copyObj)
{
    pulsanteCopia=copyObj;
    pulsanteId=copyObj->pulsanteId;
    pulsanteIndex=copyObj->pulsanteIndex;
    pulsanteState=copyObj->pulsanteState;
    parentWindow = parent;

    pulsanteDisabledPix = 0;
    pulsanteNotActivatedPix = 0;
    pulsanteActivatedPix = new QPixmap(activatedPix);
    pulsantePath = new QPainterPath(path);
    pulsanteEnabled = true;

    posX=x;
    posY=y;

    // Crea Item grafico associato al pulsante
    pulsanteBound = QRectF(pulsanteActivatedPix->rect());
    pulsanteBound.moveTo(x,y);

    pulsanteItem = new GPushItem((QObject*)this);
    parentWindow->addItem(pulsanteItem);

    // Attiva la connessione per le opzioni combo ma solo se index !=0
    if(copyObj->pulsanteIndex)
    {
        connect(&GWindowRoot,SIGNAL(pushComboActivationSgn(int,int)),this,SLOT(pushComboNotify(int,int)),Qt::UniqueConnection);
    }

    // Connessione al segnale di Root per attivazione esterna
    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)),this,SLOT(pushActivationNotify(int,bool,int)),Qt::UniqueConnection);

    opt =0;

}

// Versione pulsante trasparente
GPush::GPush(GWindow* parent, QPainterPath path, int x, int y, int index, int data, bool state)
{
    pulsanteId = GPush::classId++;
    pulsanteIndex = index;
    parentWindow = parent;

    pulsanteDisabledPix = 0;
    pulsanteNotActivatedPix = 0;
    pulsanteActivatedPix = 0;
    pulsantePath = new QPainterPath(path);

    pulsanteState=state;
    pulsanteData = data;
    pulsanteEnabled=true;
    posX=x;
    posY=y;
    pulsanteCopia=0;

    // Crea Item grafico associato al pulsante
    pulsanteBound = path.boundingRect();
    pulsanteBound.moveTo(x,y);

    pulsanteItem = new GPushItem((QObject*)this);
    parentWindow->addItem(pulsanteItem);

    // Attiva la connessione per le opzioni combo ma solo se index !=0
    if(index)
    {
        connect(&GWindowRoot,SIGNAL(pushComboActivationSgn(int,int)),this,SLOT(pushComboNotify(int,int)),Qt::UniqueConnection);
    }

    // Connessione al segnale di Root per attivazione esterna
    connect(&GWindowRoot,SIGNAL(pushActivationSgn(int,bool,int)),this,SLOT(pushActivationNotify(int,bool,int)),Qt::UniqueConnection);

    // Registrazione pulsante nella classe GWindowRoot
    GWindowRoot.pushList.append((QObject*)this);

    opt =0;

}


void GPush::setPix(QString nap, QString ap)
{

    if(nap!="")
    {
        if(pulsanteNotActivatedPix) *pulsanteNotActivatedPix=QPixmap(nap);
        else pulsanteNotActivatedPix = new QPixmap(nap);

    }else
    {
        if(pulsanteNotActivatedPix)
        {
            delete pulsanteNotActivatedPix;
            pulsanteNotActivatedPix=0;
        }
    }

    if(ap!="")
    {
        if(pulsanteActivatedPix) *pulsanteActivatedPix=QPixmap(ap);
        else pulsanteActivatedPix = new QPixmap(ap);
    }else
    {
        if(pulsanteActivatedPix)
        {
            delete pulsanteActivatedPix;
            pulsanteActivatedPix=0;
        }
    }



    // Crea Item grafico associato al pulsante
    pulsanteBound = QRectF(pulsanteActivatedPix->rect());
    pulsanteBound.moveTo(posX,posY);

}

void GPush::setPix(QString ap)
{


    if(ap!="")
    {
        if(pulsanteActivatedPix) *pulsanteActivatedPix=QPixmap(ap);
        else pulsanteActivatedPix = new QPixmap(ap);
    }else
    {
        if(pulsanteActivatedPix)
        {
            delete pulsanteActivatedPix;
            pulsanteActivatedPix=0;
        }
    }

    // Crea Item grafico associato al pulsante
    pulsanteBound = QRectF(pulsanteActivatedPix->rect());
    pulsanteBound.moveTo(posX,posY);

}

// Segnale da Root per le attivazioni in Combo
void GPush::pushComboNotify(int id, int index)
{
    // Notifica di attivazione di un pulsante in combo
    // se Ã¨ il proprio index allora si disattiva
    if((id!=pulsanteId)&&(index==pulsanteIndex)&&(pulsanteState))
    {
        pulsanteState=false;
        pulsanteItem->update();
    }
}

// Segnale da classe Root per  attivazione singolo pulsante
void GPush::pushActivationNotify(int id, bool status,int opt)
{

    // Notifica di attivazione di un pulsante in combo
    // se Ã¨ il proprio index allora si disattiva
    if(id==pulsanteId)
    {
        pulsanteState=status;
        pulsanteItem->update();

        // Solo il pulsante originale lancia la richiesta di reset
        if ((status)&&(pulsanteIndex)&&(pulsanteCopia==0)) GWindowRoot.comboResetReq(pulsanteId, pulsanteIndex); // Se il pulsante Ã¨ in combo..
    }
}

void GPush::pushResetCombo(int code, int opt)
{
    GWindowRoot.comboResetReq(0, code);
}

void  GPush::activate(bool status,int opt)
{
    GWindowRoot.pushActivate(pulsanteId, status,opt);
}

// Funzione di classe statica
void GPush::pushActivate(int id, bool status,int opt)
{
    GWindowRoot.pushActivate(id, status, opt);
}

// Funzione di classe statica
void GPush::pushRefresh(int opt)
{
    GPush* ppush;

    for(int ciclo=0; ciclo<GWindowRoot.pushList.size(); ciclo++)
    {
        ppush = ((GPush*)GWindowRoot.pushList.at(ciclo));
        GWindowRoot.pushActivate(ppush->pulsanteId, ppush->pulsanteState,opt);
    }
}

void GPush::setEnable(bool ena)
{
    if(ena)
    {
        pulsanteEnabled=true;

    }else
    {
        //pulsanteState=false;
        pulsanteEnabled=false;
    }
    pulsanteItem->update();
}

/*
 *  Questa funzione attiva o disattiva la presenza del pulsante
 *  nell'interfaccia. Quando disattivata, il pulsante non genera
 *  nessun segnale e non intercetta il mouse.
 */
void GPush::setVisible(bool ena)
{
    if(ena) this->pulsanteItem->show();
    else this->pulsanteItem->hide();
}

GLabel::GLabel(GWindow* parent, QRectF bound, QFont font, QColor color,QString testo, Qt::AlignmentFlag flags)
{

    parentWindow=parent;
    labelBound = bound;
    labelColor= color;
    labelFont = font;
    labelText = testo;
    alignmentText=flags;
    parent->addItem(this);
    update();
}

GLabel::GLabel(GWindow* parent, QRectF bound, QFont font, QColor color, char* testo, Qt::AlignmentFlag flags)
{

    parentWindow=parent;
    labelBound = bound;
    labelColor= color;
    labelFont = font;
    labelText = QString::fromUtf8(testo);
    alignmentText=flags;
    parent->addItem(this);
    update();
}

// Versione non associata a GWindow
GLabel::GLabel(QGraphicsScene* parent, QRectF bound, QFont font, QColor color,QString testo, Qt::AlignmentFlag flags)
{
    labelBound = bound;
    labelColor= color;
    labelFont = font;
    labelText = testo;
    alignmentText=flags;
    parent->addItem(this);
    update();
}



// FUNZIONE CHIAMATA IN CASO DI PAINT
void GLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,  QWidget *widget)
{

    QPen pen;

    // DIDASCALIA
    pen.setColor(labelColor);
    painter->setPen(pen);
    painter->setFont(labelFont);
    painter->drawText(labelBound,alignmentText,labelText,NULL);

}


void GLabel::setPlainText(char* testo)
{
    labelText = QString(testo);//QString::fromUtf8(testo);
}

void GLabel::setVisible(bool stat)
{
    if(stat) this->show();
    else this->hide();
}
