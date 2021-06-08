#ifndef AEC_H
#define AEC_H

#include "application.h"




class AEC
{


public:
    explicit AEC(QString tubedir);

    typedef struct {
        int pmmi;           // mm di plexifglass corrispondente
        unsigned char plog; // plog di calibrazione
        int pulse;          // Impulsi di calibrazione
    }profilePoint_Str;

    // Utilizzato nella configurazione del generatore in caso di macchine analogiche
    typedef struct{
        QString filename;                       // File name + Abolute Path
        QString basefilename;                   // File name senza estensione
        QString symbolicName;                   // Nome simbolico profilo
        QString note;                           // Stringa con note dell'esecutore

        unsigned char   plateType;              // [0:1]
        unsigned char   technic;                // [0:2]    Selezione tra STD, LD, HC
        int             plog_threshold;           // Soglia di transizione filtro automatico
        int             plog_minimo;              // Minimo valore di plog accettabile
        int             odindex;                  // Indice corrente di od selezionato
        int             od[11];                   // Correzione percentuale di annerimento (il 5 sta in mezzo)

        // Lista degli impulsi di calibrazione
        QList<profilePoint_Str> pulseStd_Mo_G;
        QList<profilePoint_Str> pulseStd_Mo_P;
        QList<profilePoint_Str> pulseStd_Rh_G;
        QList<profilePoint_Str> pulseStd_Rh_P;

    }profileCnf_Str;


public:
    QString parentDirectory; // Directory associata al Tubo selezionato

    QList<profileCnf_Str> profileList;  // In caso di macchine analogiche, vengono gestiti i profili AEC
    QList<QString> validList;           // Lista files ordinata

    bool aec_configured;               // Profili correttamente letti e configurati

    // FUNZIONI DEDICATE ALLA GESTIONE DELLA MACCHINA ANALOGICA
    bool ordinaProfili(QList<profilePoint_Str>* ptr);
    profileCnf_Str getProfile(QString filename);
    QString getCurrentProfileSymName(void);
    bool isCurrentProfileCalibrated(QString filtro, int size);
    bool isCurrentProfileCalibrated(int filtro, int size);
    int getMinPlog(void);
    void eraseProfile(QString filename);
    void eraseAllProfiles(void);
    bool openAECProfiles(void);
    bool saveSelectedProfile(void);
    void saveProfile(AEC::profileCnf_Str profile);
    profileCnf_Str* getCurrentProfilePtr(void);
    profileCnf_Str* getNextProfilePtr(void);
    profileCnf_Str* getPrevProfilePtr(void);
    profileCnf_Str* selectProfile(unsigned char index);
    int             selectProfile(QString name);
    int             getNumProfiles(void);


    QFileInfoList   getProfileList(void);   // Restituisce la lista completa dei file profilo con relativo path assoluto.
    void            getValidList(void);                // Carica la lista di validazione
    void            eraseValidList(void);              // Cancella il file di validazione; true se il file esisteva
    void            saveValidList(QList<QString> list);
    void            addFileToValidList(QString filename);


    int getAecData(int plog, int modo_filtro,  int selected_filtro, int odindex, int techmode, QString selectedAnodo, int selectedFSize, int* filtro,float* kV, int* dmAs, int* pulses);

};

#endif // AEC_H
