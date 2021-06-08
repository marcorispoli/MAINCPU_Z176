#include "application.h"
#include "appinclude.h"
#include "globvar.h"


protoConsole::protoConsole(int i,bool unicode)
{
    isUnicode = unicode;
    isValid =  TRUE;
    id = i;
    parametri.clear();
    len = 0;
    return;
}

// Funzione di ricezione del buffer di Byte in formato Unicode
// Su questo protocollo non si può innestare il formato FTP
protoConsole::protoConsole(QString* frame)
{
    int i,init;
    QString param;
    QChar terminator;

    isValid=FALSE;
    isFtpProtocol=FALSE;
    frame_len = 0;
    len =0;
    isUnicode = true;

    // Riconoscimento protocollo
    //QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
    //QString frame = codec->toUnicode(array);

    // Inizio sequenza: carattere '<'
    for(i=0; i<frame->size(); i++)
    {
        // Elimina gli spazi vuoti
        if(frame->at(i)=='<') break;
    }
    if(i==frame->size()) return;
    init = i; // Init frame index

    // Ricerca ID
    param.clear();
    while(++i<frame->size())
    {
        // Elimina gli spazi vuoti
        if(frame->at(i)==' ') continue;
        if((frame->at(i)>'9')||(frame->at(i)<'0')) return;

        // Costruisce ID
        while(i<frame->size())
        {
            if(frame->at(i)==' ') break;
            if((frame->at(i)>'9')||(frame->at(i)<'0')) return;
            param.append(frame->at(i));
            i++;
        }
        break;
    }
    if(i==frame->size()) return;
    id=param.toInt();

    // Ricerca LEN
    param.clear();
    while(++i<frame->size())
    {
        // Elimina gli spazi vuoti
        if(frame->at(i)==' ') continue;
        if((frame->at(i)>'9')||(frame->at(i)<'0')) return;

        // Costruisce ID
        while(i<frame->size())
        {
            if(frame->at(i)==' ') break;
            if((frame->at(i)>'9')||(frame->at(i)<'0')) return;
            param.append(frame->at(i));
            i++;
        }
        break;
    }
    if(i==frame->size()) return;
    len=param.toInt();

    // Ricerca Inizio corpo del messaggio tra % %
    while(++i<frame->size())
    {
        // Elimina gli spazi vuoti
        if(frame->at(i)=='%') break;
        if(frame->at(i)!=' ') return;
    }
    if(i==frame->size()) return;

    // Ricerca Comando
    param.clear();
    while(++i<frame->size())
    {
        // Elimina gli spazi vuoti
        if(frame->at(i)==' ') continue;
        if(frame->at(i)=='%') return;
        if(frame->at(i)=='>') return;

        // Costruisce Comando
        while(i<frame->size())
        {
            if(frame->at(i)==' ') break;
            if(frame->at(i)=='%') break;
            if(frame->at(i)=='>') return;
            param.append(frame->at(i));
            i++;
        }
        break;
    }

    if(i==frame->size()) return;
    comando=param;
    i--;

    // Ricerca tutti i parametri
    while(++i<frame->size())
    {
        if(frame->at(i)=='%') break;     // Fine ricerca parametri
        if(frame->at(i)==' ') continue;  // Elimina spazi
        if(frame->at(i)=='>') return;
        param.clear();

        // Cerca gli apici per determinare una stringa unica
        if(frame->at(i)=='"')
        {
            terminator='"';
            i++;
        }else  terminator = ' ';

        while(i<frame->size())
        {
            if(frame->at(i)==terminator) break;
            if(frame->at(i)=='%') break;
            if(frame->at(i)=='>')
            {
                parametri.clear();
                return;
            }

            param.append(frame->at(i));
            i++;
        }
        parametri.append(param);
        if(terminator==' ') i--;
    }
    if(i==frame->size())
    {
        parametri.clear();
        return;
    }

    // Verifica fine sequenza: simbolo '>'
    while(++i<frame->size())
    {
        if(frame->at(i)!=' ') break;
    }
    if(frame->at(i)!='>')
    {
        parametri.clear();
        return;
    }

    // Verifica lunghezza totale
    frame_len = i-init+1;
    if(frame_len !=len)
    {
        parametri.clear();
        return;
    }

    isValid=TRUE;
    return;

}
// Funzione di ricezione del buffer di Byte e trasformazione
// in Unicode
protoConsole::protoConsole(QByteArray* frame)
{
    int i,init;
    QByteArray param;
    char terminator;

    isValid=FALSE;
    isFtpProtocol=FALSE;
    errorCrc = false;
    frame_len = 0;
    len =0;

    isUnicode = false;

    // Riconoscimento protocollo
    //QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
    //QString frame = codec->toUnicode(array);

    // Inizio sequenza: carattere '<'
    for(i=0; i<frame->size(); i++)
    {
        // Elimina gli spazi vuoti
        if(frame->at(i)=='<') break;
    }
    if(i==frame->size()) return;
    init = i; // Init frame index

    // Se il carattere successivo è # allora trattasi di FTP
    if(frame->at(i+1)=='#')
    {
        isFtpProtocol = true;
        ftp.clear();
        i+=2;
        if(frame->size()==i) return;
        ftpCrc = (unsigned char) frame->at(i);
        unsigned char crc = 0;
        i++;

        // Copia tutto il resto
        for(;i<frame->size();i++){
            ftp.append(frame->at(i));
            crc = crc ^ (unsigned char) frame->at(i);
        }
        if(crc != ftpCrc )
        {
            errorCrc=true;
            return;
        }
        return;
    }

    // Ricerca ID
    param.clear();
    while(++i<frame->size())
    {
        // Elimina gli spazi vuoti
        if(frame->at(i)==' ') continue;
        if((frame->at(i)>'9')||(frame->at(i)<'0')) return;

        // Costruisce ID
        while(i<frame->size())
        {
            if(frame->at(i)==' ') break;
            if((frame->at(i)>'9')||(frame->at(i)<'0')) return;
            param.append(frame->at(i));
            i++;
        }
        break;
    }
    if(i==frame->size()) return;
    id=param.toInt();

    // Ricerca LEN
    param.clear();
    while(++i<frame->size())
    {
        // Elimina gli spazi vuoti
        if(frame->at(i)==' ') continue;
        if((frame->at(i)>'9')||(frame->at(i)<'0')) return;

        // Costruisce ID
        while(i<frame->size())
        {
            if(frame->at(i)==' ') break;
            if((frame->at(i)>'9')||(frame->at(i)<'0')) return;
            param.append(frame->at(i));
            i++;
        }
        break;
    }
    if(i==frame->size()) return;
    len=param.toInt();

    // Ricerca Inizio corpo del messaggio tra % %
    while(++i<frame->size())
    {
        // Elimina gli spazi vuoti
        if(frame->at(i)=='%') break;
        if(frame->at(i)!=' ') return;
    }
    if(i==frame->size()) return;

    // Ricerca Comando
    param.clear();
    while(++i<frame->size())
    {
        // Elimina gli spazi vuoti
        if(frame->at(i)==' ') continue;
        if(frame->at(i)=='%') return;
        if(frame->at(i)=='>') return;

        // Costruisce Comando
        while(i<frame->size())
        {
            if(frame->at(i)==' ') break;
            if(frame->at(i)=='%') break;
            if(frame->at(i)=='>') return;
            param.append(frame->at(i));
            i++;
        }
        break;
    }
    if(i==frame->size()) return;
    comando=param;
    i--;

    // Ricerca tutti i parametri
    while(++i<frame->size())
    {
        if(frame->at(i)=='%') break;     // Fine ricerca parametri
        if(frame->at(i)==' ') continue;  // Elimina spazi
        if(frame->at(i)=='>') return;
        param.clear();

        // Cerca gli apici per determinare una stringa unica
        if(frame->at(i)=='"')
        {
            terminator='"';
            i++;
        }else  terminator = ' ';

        while(i<frame->size())
        {
            if(frame->at(i)==terminator) break;
            if(frame->at(i)=='%') break;
            if(frame->at(i)=='>')
            {
                parametri.clear();
                return;
            }

            param.append(frame->at(i));
            i++;
        }
        parametri.append(param);        
        if(terminator==' ') i--;
    }
    if(i==frame->size())
    {
        parametri.clear();
        return;
    }

    // Verifica fine sequenza: simbolo '>'
    while(++i<frame->size())
    {
        if(frame->at(i)!=' ') break;
    }
    if(frame->at(i)!='>')
    {
        parametri.clear();
        return;
    }

    // Verifica lunghezza totale
    frame_len = i-init+1;
    if(frame_len !=len)
    {
        parametri.clear();
        return;
    }

    isValid=TRUE;
    return;

}

QByteArray protoConsole::createFtpArray(QByteArray data)
{

    // Calcola il crc
    unsigned char crc=0;
    for(int i =0; i<data.size(); i++)
        crc ^= (unsigned char) data.at(i);
    data.prepend(crc);
    data.prepend("<#");
    this->ftpCrc = crc;
    return data;
}

void protoConsole::addParam(QString par)
{
    parametri.append(par);
    return;
}

QByteArray protoConsole::cmdToQByteArray(QString cmd)
{
    QByteArray data;
    QString par,ID,LEN,frame;
    int i;

    par.clear();
    par.append("%");
    par.append(cmd);
    comando=cmd;
    par.append(" ");
    if(parametri.count())
    {
        for(i=0; i<parametri.count(); i++)
        {
            par.append(parametri[i]);
            par.append(" ");
        }
    }
    par.append("%>");


    ID=QString("<%1 ").arg(id);
    len = ID.size()+par.size()+3;

    while(1)
    {
        LEN=QString("%1").arg(len);
        frame=QString("%1%2 %3").arg(ID).arg(LEN).arg(par);

        if(frame.size()==len) break;
        len = frame.size();
    }

    if(isUnicode){
        QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
        return codec->fromUnicode(frame);
    }else  return frame.toAscii();
}

QByteArray protoConsole::answToQByteArray(QString cmd)
{
    return cmdToQByteArray(cmd);
}

QByteArray protoConsole::answToQByteArray(void)
{
    return cmdToQByteArray(QString("OK"));
}

QByteArray protoConsole::answToQByteArray(int time)
{
    addParam(QString("%1").arg(time));
    return cmdToQByteArray(QString("OK"));
}


QByteArray protoConsole::ackToQByteArray(int code)
{
    QString frame;

    len = 14;
    while(1)
    {
        frame=QString("<A %1 % %2 %3 %>").arg(len).arg(id).arg(code);
        if(frame.size()==len) break;
        len = frame.size();
    }

    if(isUnicode){
        QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
        return codec->fromUnicode(frame);
    }else  return frame.toAscii();
}

// Costruisce il frame sostituendo da comd tutti i caratteri speciali utilizzati
// per definire il protocollo: <>%
QByteArray protoConsole::formatToQByteArray(QString frame){
    frame.replace("<","#");
    frame.replace(">","@");
    frame.replace("%","$");

    if(isUnicode){
        QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
        return codec->fromUnicode(frame);
    }else  return frame.toAscii();
}

QByteArray protoConsole::unformatData(QString cmd){
    cmd.replace("#","<");
    cmd.replace("@",">");
    cmd.replace("$","%");
    if(isUnicode){
        QTextCodec *codec = QTextCodec::codecForName(UNICODE_TYPE);
        return codec->fromUnicode(cmd);
    }else  return cmd.toAscii();
}
