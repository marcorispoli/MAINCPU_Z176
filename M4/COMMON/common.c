#define _COMMON_C
#include "dbt_m4.h"

// ATTENDE l'evento per un massimo di tot ms
// FALSE = TIMEOUT
bool msEventWaitAll(int mask, LWEVENT_STRUCT* ev, unsigned int ms)
{
    MQX_TICK_STRUCT ticks;

    _time_init_ticks(&ticks, 0);
    _time_add_msec_to_ticks(&ticks,ms);
    
    if(_lwevent_wait_for(ev, mask, TRUE, &ticks)==LWEVENT_WAIT_TIMEOUT)
      return FALSE;
    
    return TRUE;
}

bool msEventWaitAny(int mask, LWEVENT_STRUCT* ev, unsigned int ms)
{
    MQX_TICK_STRUCT ticks;

    _time_init_ticks(&ticks, 0);
    _time_add_msec_to_ticks(&ticks,ms);
    
    if(_lwevent_wait_for(ev, mask, FALSE, &ticks)==LWEVENT_WAIT_TIMEOUT)
      return FALSE;
    
    return TRUE;
}

/*
    FUNZIONE UTILIZZATA PER L'iMPOSTAZIONE DEGLI OUTPUTS  DI SISTEMA
*/
void setOutputs(_SystemOutputs_Str* pSet, _SystemOutputs_Str* pMask)
{
  
    int i;
    unsigned char new,mask,prev;
    bool changed=FALSE;

    // Blocca la mutex sul calcolo
    _mutex_lock(&output_mutex);    
        for(i=0; i<sizeof(SystemOutputs); i++)
        {
            mask = ((unsigned char*)pMask)[i];
            new = mask & ((unsigned char*)pSet)[i];
            if( (mask&((unsigned char*)&SystemOutputs)[i]) ^ new) changed = TRUE;
            ((unsigned char*)&SystemOutputs)[i] &= ~mask; // Toglie la maschera
            ((unsigned char*)&SystemOutputs)[i] |= new;   // Aggiunge il nuovo dato
        }
        if(changed) _EVSET(_EV0_OUTPUT_CAMBIATI);
     _mutex_unlock(&output_mutex);
}




int nearest(float v){
  int val = (int) v;
  if((v-(float) val)>0.5) return val+1;
  else return val;    
}
/* EOF */
