#define _MCCCLASS_CPP
#include "dbt_m4.h"
#include "../../shared_a5_m4/debug_print.h"

#ifdef M4_MASTER
MCC_ENDPOINT	src_ep={_DEF_MCC_GUI_TO_M4_MASTER};
#else
MCC_ENDPOINT	src_ep={_DEF_MCC_GUI_TO_M4_MASTER};
#endif 

static bool mccLibNotify(unsigned char ncode, unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen);

bool mccOpenPort(MCC_ENDPOINT *ep)
{
  if(mcc_create_endpoint(ep,ep->port)!=MCC_SUCCESS)
    return FALSE;
  return TRUE;
}

////////////////////////////////////////////////////////////////////
/*
 *  Routine di ricezione ed accettazione Frame
 *  La routine è bloccante
 *
 */
////////////////////////////////////////////////////////////////////
bool mccRxFrame(MCC_ENDPOINT* ep, _MccFrame_Str* mcc_cmd)
{
  MCC_MEM_SIZE mcc_len;

   mcc_len = 0;
   if(mcc_recv_copy(ep,mcc_cmd,sizeof(_MccFrame_Str),(MCC_MEM_SIZE*) &mcc_len,0xffffffff)!=MCC_SUCCESS)
      return FALSE;
   if(mcc_len==0) return FALSE;
   
  
    return mccIsFrameCorrect(mcc_cmd);
}   
    

////////////////////////////////////////////////////////////////////
/*
 *  Aggiunge il checksum al buffer del messaggio
 *
 */
////////////////////////////////////////////////////////////////////
unsigned char mccSetChecksum(_MccFrame_Str* mcc_cmd)
{
    int i;
    mcc_cmd->checksum=0;
    for(i=0;i<mcc_cmd->len;i++)
        mcc_cmd->checksum ^= mcc_cmd->buffer[i];

    return mcc_cmd->checksum;
}

////////////////////////////////////////////////////////////////////
/*
 *  Prepara ed invia il messaggio da inviare a M4 secondo il protocollo
 *  interno
 *
 */
////////////////////////////////////////////////////////////////////
bool mccSendBuffer(MCC_ENDPOINT* ep, unsigned char cmd, unsigned char id, unsigned char* buffer, unsigned char len)
{
    int i;
    MCC_MEM_SIZE mcc_len;
    _MccFrame_Str mcc_cmd;
    
    // Verifica se busy
    mcc_msgs_available(ep,&mcc_len);
    if(mcc_len>MAX_MCC_QUEUE){      
        DEBUG_PRINT3(__DBG_MCC_FULL,id,cmd,buffer[0]);
        return FALSE;
    }

    // Controllo sulla dimensione del messaggio
    if (len > _MCC_DIM) _mqx_exit(-1);

    // Crea il pacchetto da spedire
    mcc_cmd.cmd = cmd;
    mcc_cmd.id = id;
    mcc_cmd.len = len;
    mcc_cmd.checksum = 0;
    for(i=0; i<len; i++) mcc_cmd.buffer[i] = buffer[i];
    mccSetChecksum(&mcc_cmd);

    // Invia a M4
    if(mcc_send(ep,(void*)&mcc_cmd,sizeof(_MccFrame_Str),0)!=MCC_SUCCESS)
        return FALSE;

    return TRUE;
}

bool mccSendBufferFlags(MCC_ENDPOINT* ep, unsigned char cmd, unsigned char id, unsigned char* buffer, unsigned char len,unsigned char data1, unsigned char data2)
{
    int i;
    MCC_MEM_SIZE mcc_len;
    _MccFrame_Str mcc_cmd;
    
    // Verifica se busy
    mcc_msgs_available(ep,&mcc_len);
    if(mcc_len>MAX_MCC_QUEUE){
        DEBUG_PRINT3(__DBG_MCC_FULL,id,cmd,buffer[0]);
        return FALSE; // C'è già un messaggio in coda
    }

    // Controllo sulla dimensione del messaggio
    if (len > _MCC_DIM) _mqx_exit(-1);

    // Crea il pacchetto da spedire
    mcc_cmd.cmd = cmd;
    mcc_cmd.id = id;
    mcc_cmd.len = len+2;
    mcc_cmd.checksum = 0;
    for(i=0; i<len; i++) mcc_cmd.buffer[i] = buffer[i];
    mcc_cmd.buffer[i] = data1;
    mcc_cmd.buffer[i+1] = data2;
    mccSetChecksum(&mcc_cmd);
    // Invia a M4
    if(mcc_send(ep,(void*)&mcc_cmd,sizeof(_MccFrame_Str),0)!=MCC_SUCCESS)
        return FALSE;

    return TRUE;
}

////////////////////////////////////////////////////////////////////
/*
 *  invia un messaggio già formattato
 *
 */
////////////////////////////////////////////////////////////////////
bool mccSendFrame(MCC_ENDPOINT* ep, _MccFrame_Str* mcc_cmd)
{
    int i;
    MCC_MEM_SIZE mcc_len;
    
    // Verifica se busy
    mcc_msgs_available(ep,&mcc_len);
    if(mcc_len>MAX_MCC_QUEUE){
        DEBUG_PRINT3(__DBG_MCC_FULL,mcc_cmd->id,mcc_cmd->cmd,mcc_cmd->buffer[0]);
        return FALSE; // C'è già un messaggio in coda
    }
 
    // Invia a M4
    if(mcc_send(ep,(void*) mcc_cmd,sizeof(_MccFrame_Str),0)!=MCC_SUCCESS)
        return FALSE;

    return TRUE;
}

////////////////////////////////////////////////////////////////////
/*
 *  Verifica se il messaggio è corretto
 */
////////////////////////////////////////////////////////////////////
bool mccIsFrameCorrect(_MccFrame_Str* mccframe)
{
    int i;
    unsigned char chk=0;

    if(mccframe->len>_MCC_DIM) return FALSE;

    for(i=0;i<mccframe->len;i++)
        chk^=mccframe->buffer[i];
      
    if(chk!=mccframe->checksum) return FALSE;
    return TRUE;
}

/*______________________________________________________________________________
    HANDLER PER NOTIFICHE AL PROCESSO CHIAMANTE
PARAMETRI:
    mcccode: Codice MCC dichiarato all'applicazione
    id = ID ricevuto dal chiamante;
  
    buffer = puntatore al buffer da inviare
    buflen = numero byte da inviare

  ATTENZIONE: Se id==0 non verrà inviata nessuna notifica all'applicazione
*/

bool mccLibNotify(unsigned char ncode, unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen)
{      
    unsigned char data[_MCC_DIM];
    
#ifdef M4_MASTER

    MCC_ENDPOINT ep = {_DEF_MCC_MASTER_TO_APP_MASTER};
#else
    MCC_ENDPOINT ep = {_DEF_M4_SLAVE_TO_APP_SLAVE};
#endif
    
    // Nessuna notifica se ID == 0
    if(id==0){      
      return TRUE;
    }
    if(buflen>_MCC_DIM-2)
    {
      // Errore ispezionato durante lo sviluppo
      printf("(MCC NOTIFY LIB: %d): BUFLEN SUPERIORE AL MASSIMO CONSENTITO",mcccode);      
      return FALSE;
    }
       
    data[0]=mcccode;
    data[1]=buflen;
    memcpy(&data[2],buffer,buflen);

    // Invia all'applicazione
    bool ris =  mccSendBuffer(&ep,ncode,id, data, buflen+2);    
    return ris;
   
}          

bool mccGuiNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen)
{
    return mccLibNotify(MCC_GUI_NOTIFY,id,mcccode,buffer,buflen);
}        
bool mccServiceNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen)
{
    return mccLibNotify(MCC_SERVICE_NOTIFY,id,mcccode,buffer,buflen);
}        
bool mccLoaderNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen)
{
    return mccLibNotify(MCC_LOADER_NOTIFY,id,mcccode,buffer,buflen);
}        

bool mccPCB215Notify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen)
{
    return mccLibNotify(MCC_PCB215_NOTIFY,id,mcccode,buffer,buflen);
}          
bool mccPCB244ANotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen)
{
     return mccLibNotify(MCC_244_A_FUNCTIONS,id,mcccode,buffer,buflen);
}
/*bool mccActuatorNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen)
{
    return mccLibNotify(MCC_ACTUATOR_NOTIFY,id,mcccode,buffer,buflen);
} */
bool mccPCB249U1Notify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen)
{
    return mccLibNotify(MCC_PCB249U1_NOTIFY,id,mcccode,buffer,buflen);
}          
bool mccPCB190Notify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen)
{
    return mccLibNotify(MCC_PCB190_NOTIFY,id,mcccode,buffer,buflen);
}      
bool mccBiopsyNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen)
{
    return mccLibNotify(MCC_BIOP_NOTIFY,id,mcccode,buffer,buflen);
}          

bool mccConfigNotify(unsigned char id,unsigned char mcccode,unsigned char* buffer, int buflen)
{
    return mccLibNotify(MCC_CONFIG_NOTIFY,id,mcccode,buffer,buflen);
}          



/* EOF */
