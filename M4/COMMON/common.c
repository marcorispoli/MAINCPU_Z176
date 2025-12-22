#define _COMMON_C
#include "dbt_m4.h"

bool debugPrintEnable = FALSE;
bool printEnable = FALSE;

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

float absF(float f){
    if(f>=0) return f;
    else return -f;
}
int absI(int i){
    if(i>=0) return i;
    else return -i;
}

#define MAX_PRINT_DEBUG 100
void debugPrintEna(bool stat){
    debugPrintEnable = stat;
    if(stat ==true ) printEnable = true;
}
void printEna(bool stat){
    printEnable = stat;
}

void debugPrint(const char* buffer){
    int i,j;
    char lbuf[MAX_PRINT_DEBUG+4];

    if(!debugPrintEnable){        
        return;
    }

    lbuf[0]='\0';
    for(i=0; i<MAX_PRINT_DEBUG; i++){
        if(buffer[i]=='\0') break;
        lbuf[i] = buffer[i];
    }
    lbuf[i++]='\0';
    mccDebugPrint(lbuf, i);
}



void debugPrintIBuffer(char* destination, int* index, char* tag, int val){
    int j;
    char valbuf[11];

    // Aggiunge il tag
    j=0;
    while(1){
        if(tag[j]=='\0') break;
        destination[(*index)++] = tag[j++];
        if((*index) >= MAX_PRINT_DEBUG) return;
    }

    valbuf[0] = '\0';
    destination[(*index)++]=':';
    sprintf(valbuf,"%d",(short) val);
    j=0;
    while(1){
        if(valbuf[j]=='\0') return;
        destination[(*index)++] = valbuf[j++];
        if((*index) >= MAX_PRINT_DEBUG) return;
        if(j>=10) return;
    }

}

void debugPrintXBuffer(char* destination, int* index, char* tag, unsigned int val){
    int j;
    char valbuf[11];

    // Aggiunge il tag
    j=0;
    while(1){
        if(tag[j]=='\0') break;
        destination[(*index)++] = tag[j++];
        if((*index) >= MAX_PRINT_DEBUG) return;
    }

    valbuf[0] = '\0';
    destination[(*index)++]=':';
    destination[(*index)++]='0';
    destination[(*index)++]='x';
    sprintf(valbuf,"%x",val);
    j=0;
    while(1){
        if(valbuf[j]=='\0') return;
        destination[(*index)++] = valbuf[j++];
        if((*index) >= MAX_PRINT_DEBUG) return;
        if(j>=10) return;
    }

}

void debugPrintI(const char* buffer, int var){
    if(!debugPrintEnable){        
        return;
    }

    char lbuf[MAX_PRINT_DEBUG+4];
    int i=0;
    debugPrintIBuffer(lbuf, &i, buffer, var);lbuf[i++]='\0';
    mccDebugPrint(lbuf, i);
}
void debugPrintI2(const char* buffer1, int var1,const char* buffer2, int var2){
    if(!debugPrintEnable){        
        return;
    }

    char lbuf[MAX_PRINT_DEBUG+4];
    int i=0;
    debugPrintIBuffer(lbuf, &i, buffer1, var1);lbuf[i++]=' ';
    debugPrintIBuffer(lbuf, &i, buffer2, var2);lbuf[i++]='\0';
    mccDebugPrint(lbuf, i);
}
void debugPrintI3(const char* buffer1, int var1,const char* buffer2, int var2,const char* buffer3, int var3){
    if(!debugPrintEnable){        
        return;
    }

    char lbuf[MAX_PRINT_DEBUG+4];
    int i=0;
    debugPrintIBuffer(lbuf, &i, buffer1, var1);lbuf[i++]=' ';
    debugPrintIBuffer(lbuf, &i, buffer2, var2);lbuf[i++]=' ';
    debugPrintIBuffer(lbuf, &i, buffer3, var3);lbuf[i++]='\0';
    mccDebugPrint(lbuf, i);
}
void debugPrintI4(const char* buffer1, int var1,const char* buffer2, int var2,const char* buffer3, int var3,const char* buffer4, int var4){
    if(!debugPrintEnable){        
        return;
    }

    char lbuf[MAX_PRINT_DEBUG+4];
    int i=0;
    debugPrintIBuffer(lbuf, &i, buffer1, var1);lbuf[i++]=' ';
    debugPrintIBuffer(lbuf, &i, buffer2, var2);lbuf[i++]=' ';
    debugPrintIBuffer(lbuf, &i, buffer3, var3);lbuf[i++]=' ';
    debugPrintIBuffer(lbuf, &i, buffer4, var4);lbuf[i++]='\0';
    mccDebugPrint(lbuf, i);

}

void debugPrintX(const char* buffer, unsigned int var){
    if(!debugPrintEnable){        
        return;
    }

    char lbuf[MAX_PRINT_DEBUG+4];
    int i=0;
    debugPrintXBuffer(lbuf, &i, buffer, var);lbuf[i++]='\0';
    mccDebugPrint(lbuf, i);
}
void debugPrintX2(const char* buffer1, unsigned int var1,const char* buffer2, unsigned int var2){
    if(!debugPrintEnable){        
        return;
    }

    char lbuf[MAX_PRINT_DEBUG+4];
    int i=0;
    debugPrintXBuffer(lbuf, &i, buffer1, var1);lbuf[i++]=' ';
    debugPrintXBuffer(lbuf, &i, buffer2, var2);lbuf[i++]='\0';
    mccDebugPrint(lbuf, i);
}
void debugPrintX3(const char* buffer1, unsigned int var1,const char* buffer2, unsigned int var2,const char* buffer3, unsigned int var3){
    if(!debugPrintEnable){        
        return;
    }

    char lbuf[MAX_PRINT_DEBUG+4];
    int i=0;
    debugPrintXBuffer(lbuf, &i, buffer1, var1);lbuf[i++]=' ';
    debugPrintXBuffer(lbuf, &i, buffer2, var2);lbuf[i++]=' ';
    debugPrintXBuffer(lbuf, &i, buffer3, var3);lbuf[i++]='\0';
    mccDebugPrint(lbuf, i);
}
void debugPrintX4(const char* buffer1, unsigned int var1,const char* buffer2, unsigned int var2,const char* buffer3, unsigned int var3,const char* buffer4, unsigned int var4){
    if(!debugPrintEnable){        
        return;
    }

    char lbuf[MAX_PRINT_DEBUG+4];
    int i=0;
    debugPrintXBuffer(lbuf, &i, buffer1, var1);lbuf[i++]=' ';
    debugPrintXBuffer(lbuf, &i, buffer2, var2);lbuf[i++]=' ';
    debugPrintXBuffer(lbuf, &i, buffer3, var3);lbuf[i++]=' ';
    debugPrintXBuffer(lbuf, &i, buffer4, var4);lbuf[i++]='\0';
    mccDebugPrint(lbuf, i);

}

void debugPrintF(const char* buffer, float var){
    int i,j;
    char lbuf[MAX_PRINT_DEBUG+4];
    char valbuf[11];

    if(!debugPrintEnable){        
        return;
    }

    lbuf[0]='\0';
    for(i=0; i<MAX_PRINT_DEBUG; i++){
        lbuf[i] = buffer[i];
        if(buffer[i]=='\0') break;
    }
    if(i==MAX_PRINT_DEBUG){
        lbuf[i++]='\0';
         mccDebugPrint(lbuf, i);
         return;
    }

    lbuf[i++]=':';

    sprintf(valbuf,"%f",var);

    for(j=0;j<10; j++){
        lbuf[i++] = valbuf[j];
        if(valbuf[j]=='\0') break;
        if(i==MAX_PRINT_DEBUG) break;
    }
    lbuf[i++]='\0';
    mccDebugPrint(lbuf, i);
}
