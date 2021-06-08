/*

Aut: M. Rispoli
Data: 3/10/2014
Data Modifica:
*/
#ifndef _FAULT_CODE
#define _FAULT_CODE

#ifdef ext
#undef ext
#undef extrd
#endif
#ifdef _FAULT_C
  #define ext 
  #define extrd 
#else
  #define ext extern
  #define extrd extern const
#endif

typedef enum
{
  _FLT_PCB_204_DRIVER=0,
  _FLT_PCB_215_DRIVER,
}_faultSource_Enum;
  
typedef struct 
{
  _faultSource_Enum source;   // Tipologia di errore
  char* buffer;               // Buffer di caratteri per eventuali sringhe aggiuntive
  _task_id    tid;            // Task ID del processo chiamante
  _mqx_uint   prio;           // Priorità originale del processo chiamante da ripristinare
  bool blocco;                // Modalità processo bloccato e cambio priorità
}_faultStat_Str;


/////////////////////////////////////////////////////////////////////////
  
ext _faultStat_Str faultStat; // Struttura per la definizione dell'errore
 
// API ////////////////////////////////////////////////////////////////// 

ext void fault_task(uint32_t initial_data); // Task di gestione faults
void setFault(_faultSource_Enum src, char* buf, bool blk_mode);
  
// MACRO ////////////////////////////////////////////////////////////////// 

#endif