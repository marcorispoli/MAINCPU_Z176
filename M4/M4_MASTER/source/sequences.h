/*

Aut: M. Rispoli
Data di Creazione: 30/09/2014

Data Ultima Modifica:2/10/2014
*/
#ifndef _SEQUENCES
#define _SEQUENCES

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _SEQUENCES_C
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif

 ext void analog_rx_task(uint32_t initial_data);

#define _WAIT_UPD               2000       // ms di attesa per l'aggiornamento della configurazione
#define _WAIT_FREEZED           2000       // ms di attesa per freezed PCB190
#define _WAIT_XRAY_COMPLETED    100000     // ms attesa fine rsequenza raggi
#define _WAIT_DEVICE_RUN        2000       // ms di attesa per la ripresa del driver
#define _WAIT_XRAY_ENA          10000      // ms Di attesa aggiornamento segnale su BUS
#define _WAIT_EXP_WIN           10000
#define _WAIT_BUSY_HOME         5000


 
// Parametri di Esposizione
typedef struct 
{
  unsigned short HV;    // Valore di tensione raw
  unsigned short I;     // Valore di corrente raw
  unsigned char  MINHV; // Valore minimo durante esposizione
  unsigned char  MAXHV; // Valore massimo durante esposizione 
  unsigned char  MINI;  // Valore minimo I durante esposizione
  unsigned char  MAXI;  // Valore massimo I durante esposizione 
  unsigned short MAS;   // Valore Mas
  unsigned char  TMO;   // Timeout esposizione
  unsigned char  CHK;   // Checksum Parametri di esposizione    
}_RxParam_Str;

typedef struct
{
  _RxParam_Str   esposizione;        // Parametri di esposizione per PCB190 (sotto checksum)
  _RxParam_Str   esposizioneAE;      // Parametri di esposizione per PCB190 (sotto checksum) per AE
  unsigned char  config;             // Configurazione sequenza
  unsigned char  potter_cfg;         // Impostazione potter: 0=spento; 1=TOMO; 2=2D

  bool isAEC; // Discrimina se la seqeunza è AEC

  // Compressore
  bool compressor_unlock;             // Verifica se deve decomprimere dopo esame

  // In caso di Tomo
  unsigned char tomo_pre_pulses;    // Numero di impulsi da scartare
  unsigned char tomo_samples;       // Numero totale di impulsi
  unsigned short tomo_mode;         // Modalità Narrow/Wide
  unsigned short tomo_skip;         // Impulsi da saltare
  bool tomo_deadman;                // Funzione dead man attiva

  // Sezione riservata per sequenze analogiche
  unsigned char guiError;           // Codice di errore comunicato dalla gui
  unsigned char analog_sequence;    // Codice sequenza in corso
  unsigned char mcc_code;           // Codice per notifica GUI
  unsigned short dmAs_pre_released; // Indica solo se il pre è stato eseguito

  unsigned short dmAs_released;     // mAs rilasciati durante la sequenza
  unsigned short pulses;            // Impulsi per l'esposimetro
  unsigned short pulses_released;   // Impulsi effettivi
  unsigned short In;                // Corrente nominale
  unsigned short mAs_nom;               // mAs nominali


 }_RxStdSeq_Str;

// VALIDI PER TUTTE LE SEQUENZE
ext bool aecIsValid;
ext bool aecExpIsValid;

// SEQUENZA TOMO STANDARD 
extern _RxStdSeq_Str tomoParam;
extern bool tomoSeqResult;
extern unsigned char tomoError;
extern bool tomoIsRunning;


// SEQUENZA RAGGI STANDARD
extern _RxStdSeq_Str rxStdParam;
extern bool rxStdSeqResult;
extern bool rxStdIsRunning;
extern unsigned char rxStdError;


ext void rxNotifyData(unsigned char type, unsigned char code);

// Variabili per gestire  i campionamenti delle grandezze durante le esposizioni
ext unsigned short rx_tmed_pre; // Tempo medio esposizione
ext unsigned short rx_tmed_pls; // Tempo medio esposizione
ext unsigned short mAs_erogati;// AmAs 2D o a bassa energia
ext unsigned short mAs_erogati_AE; // Massa ad alta energia
ext unsigned short mAs_pre; // mAs erogati durante la pre-esposizione
ext unsigned char  rx_imean; // Calcolo corrente anodica media
ext unsigned char  rx_vmean; // Calcolo tensione anodica media


#define _POTTER_TOMO            1
#define _POTTER_2D              2
#define _POTTER_NO_GRID         0



#endif
