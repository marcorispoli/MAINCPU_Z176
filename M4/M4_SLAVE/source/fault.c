#define _FAULT_C
#include "dbt_m4.h"

// Funzioni Locali

static void faultResumeTask();

// Registri privati
static MUTEX_STRUCT faultSig_mutex;

//////////////////////////////////////////////////////////////////////////////
/*
void fault_task(uint32_t initial_data)



Autore: M. Rispoli
Data: 03/10/2014
*/
//////////////////////////////////////////////////////////////////////////////
void fault_task(uint32_t initial_data)
{
  if (_mutex_init(&faultSig_mutex, NULL) != MQX_OK)
  {
    printf("ERRORE NEL TASK DI GESTIONE ERRORI!! MUOIO!\n");
    _mqx_exit(-1);
  }

  while(1)
  {
    // Attende un eventuale fault per gestirlo
    _EVWAIT_ANY(_EV0_SYSTEM_FAULT);
    _EVCLR(_EV0_SYSTEM_FAULT);
    
    if(faultStat.buffer!=NULL)
      printf("SEGNALAZIONE DI FAULT TIPO:%s\n",faultStat.buffer);
    else
      printf("FAULT ANONIMO\n");
    
    
    // Discrimina il tipo di Fault che � stato segnalato e lo gestisce
    switch(faultStat.source)
    {
    default:
      printf("FAULT ANONIMO\n");
    }
  
    // Ripristina la Mutex per il prossimo fault
    _mutex_unlock(&faultSig_mutex);

  }
  
}

//////////////////////////////////////////////////////////////////////////////
/*
void setFault(_faultSource_Enum src, unsigned char id, unsigned char code, char* buf)

  Questa funzione permette di attivare una condizione di fault
  che verrà conseguentemente gestita dall'apposita thread.
  
  La funzione viene eseguita nello spazio della thread chiamante :
  la thread chiamante viene alzata di priorità al di sopra di tutte
  le priorità dell'applicazione in modo da servire il piuù presto possibile 
  il fault; il livello originale di priorità viene salvato sulla struttura
  di Stato nel caso che tale pèrocesso possa eventualmente essere 
  ripristinato. 

  Nel caso in cui il processo venga ucciso, la sua priorità più
  alta gli consentirà di eseguire immediatamente la routine di chiusura
  con messa in sicurezza del sistema.

  Nella struttura dati verranno caricati tutti i dati necessari 
  affinchè la thread di gestione degli errori possa decidere 
  le azioni conseguenti al tipo di fault segnalato



Autore: M. Rispoli
Data: 03/10/2014
*/
//////////////////////////////////////////////////////////////////////////////
void setFault(_faultSource_Enum src, char* buf, bool blk_mode)
{
  _mqx_uint old_prio;
  
  // Impedisce chiamate multiple: solo al termine della gestione 
  // viene liberata..
  _mutex_lock(&faultSig_mutex);
  
  
  faultStat.tid = _task_get_id();       // Salva il tid del processo chiamante
  _task_get_priority(faultStat.tid, &faultStat.prio); // Salva la priorit�
  faultStat.buffer = buf;
  faultStat.source = src;
  faultStat.blocco = blk_mode; // Modalit� blocco

  // Se il processo richiede il blocco dello stesso 
  if(blk_mode)
  {
    // Eleva la priorit� del chiamante per assicurare il rapido completamento 
    // delle operazioni di gestione.
    _task_set_priority(MQX_NULL_TASK_ID,(_PRIOTASK(FAULT_TASK)-1),&old_prio);
    _task_block();
  }
  
  
  // Segnala evento di Fault per svegliare la thread
  _EVSET(_EV0_SYSTEM_FAULT);
  
  return;
}

void faultResumeTask()
{
    _mqx_uint old_prio;
    
    printf("RESUME TASK\n");
  // Imposta la priorit� originaria al processo chiamante
   _task_set_priority(faultStat.tid,faultStat.prio,&old_prio);
   _task_ready(_task_get_td(faultStat.tid));
   return;
}



/* EOF */
