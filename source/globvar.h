#ifndef GLOBVAR_H
#define GLOBVAR_H

// Variabili globali dell'applicazione
#ifdef ext
#undef ext
#endif

#ifdef MAIN_C
    #define ext
#else
    #define ext extern
#endif


ext TcpIpServer*     masterTcp;
ext TcpIpClient*     slaveTcp;
ext EchoDisplay      echoDisplay;  // Sviluppa il protocollo di mirroring tra i display
ext MainPage*        paginaMainDigital;
//ext OpenStudyPage*   paginaOpenStudyDigital;
ext ProjectionPage*  paginaProjections;
ext PageLanguages*   pagina_language;
//ext PageACR*         paginaAcr;
ext PageAlarms*      paginaAllarmi;
//ext ImagePage*       paginaImmagine;

ext mccMasterCom*    pMasterRxMcc;  // Comunicazione da Master M4 Core
ext mccSlaveCom*     pSlaveRxMcc;  // Comunicazione da Master M4 Core

ext DBase            ApplicationDatabase;
ext bool isMaster;
ext console*            pConsole;
ext protoToConsole*     pToConsole;
ext serverDebug*        pDebug;

ext Generatore*      pGeneratore;
ext mccCom*         pMcc;
ext sysIO*          io;                 // Puntatore a IO di sistema

ext msgBox*     pWarningBox;

ext Config* pConfig;
ext Collimatore* pCollimatore;
ext Compressor* pCompressore;
ext Potter* pPotter;




ext bool systemTimeUpdated;
ext Loader* pLoader;
ext biopsy* pBiopsy;

ext QString _CONSOLE_IP;
ext QString _MASTER_IP;
ext QString _SLAVE_IP;
ext QString _LOCAL_SERVICE_IP;
ext systemLog* pSysLog;


#endif // GLOBVAR_H
