#define _SEQUENCES_C
#include "dbt_m4.h"

// Funzione per notificare i dati di fine esposizione
// I dati debbono essere già statio scaricati in precedenza
// type: tipo di esposizione 0 = STD, 1= AEC, 2= TOMO, 3 = AEC TOMO
// code: codice di termine raggi
void rxNotifyData(unsigned char type, unsigned char code)
{
    unsigned char  is[PCB190_NSAMPLES+1];
    unsigned char  vs[PCB190_NSAMPLES+1];
    unsigned char  ts[PCB190_NSAMPLES_TIME+1];
    unsigned char  data[RX_DATA_LEN + 2 * PCB190_NSAMPLES + PCB190_NSAMPLES_TIME + 1];
    

    int i;
    float scarto_v = 0;
    float imean,vmean,aec_imean, aec_vmean;
    int  samples, naec;
    unsigned char addr, banco;
    unsigned short tmed_pre,tmed_pls;
    unsigned char ifil_rxend;
    
    // Se non c'è stata esposizione del tutto allor restituisce solo
    // i dati necessari e non i campionamenti
    if((!((code>LAST_ERROR_NO_PREP)&&(code<LAST_ERROR_WITH_PREP)))&&(code != RXOK)){

        data[RX_END_CODE]=code;       // Risultato esposizione
        data[MAS_PRE_L]= 0;
        data[MAS_PRE_H]= 0;
        data[MAS_PLS_L]= 0;
        data[MAS_PLS_H]= 0;
        data[I_MEAN]= 0;
        data[V_MEAN]= 0;
        data[V_SIGMA] = 0;

        data[T_MEAN_PRE_L]= 0;
        data[T_MEAN_PRE_H]= 0;
        data[T_MEAN_PLS_L]= 0;
        data[T_MEAN_PLS_H]= 0;
        data[HV_POST_RAGGI] = 0;
        data[IFIL_POST_RAGGI] = 0;

        // Carica i campionamenti dell'esposizione effettuata
        data[NSAMPLES_AEC] = 0;
        data[NSAMPLES_PLS] = 0;
        data[SAMPLES_BUFFER] = 0;

        MCC_ENDPOINT ep = {_DEF_MCC_MASTER_TO_APP_MASTER};
        mccSendBuffer(&ep,MCC_RAGGI_DATA,0, data, RX_DATA_LEN);
        return;
    }

     
    // Legge il numero di campioni AEC
    Ser422ReadRegister(_REGID(PR190_N_SAMPLE_AEC),10,&PCB190_CONTEST);
    naec = (float) _DEVREGL(PR190_N_SAMPLE_AEC,PCB190_CONTEST);
    if(naec>1) naec = 1;
    
    // Legge il numero dei campioni totali
    Ser422ReadRegister(_REGID(PR190_N_SAMPLE_I),10,&PCB190_CONTEST);
    samples = (int) _DEVREGL(PR190_N_SAMPLE_I,PCB190_CONTEST);
    if(samples > PCB190_NSAMPLES) samples = (int) PCB190_NSAMPLES;
    
    // Legge i campioni della corrente 
    addr = _DEVADDR(PR190_SAMPLES_I);
    banco = _DEVBANCO(PR190_SAMPLES_I);
    for(i=0;i<samples;i++,addr++){
        if(i>=sizeof(is)) break;
        Ser422ReadAddrRegister(addr, banco,10,&is[i], &PCB190_CONTEST);
    }

    // Legge i campioni della tensione 
    addr = _DEVADDR(PR190_SAMPLES_V);
    banco = _DEVBANCO(PR190_SAMPLES_V);
    for(i=0;i<samples;i++,addr++){
        if(i>=sizeof(vs)) break;
        Ser422ReadAddrRegister(addr, banco,10,&vs[i], &PCB190_CONTEST);
    }

    // Calcola medie AEC se ci sono campioni
    aec_imean=aec_vmean=0;
    if(naec!=0){
        for(i=0;i<naec;i++){
          printf("(AEC-%d): I[%f(mA), %d(RAW)]  V[%f(kV), %d(RAW)] \n",i,((((float) is[i])*200.)/255.), is[i],pcb190ConvertKvRead(vs[i]),vs[i]);               
          aec_imean+=(float) is[i];
          aec_vmean+=(float) vs[i];          
        }
        aec_imean=aec_imean/naec;
        aec_vmean=aec_vmean/naec;
    }
    
    // Calcola medie Impulso se ci sono campioni
    imean=vmean=0;
    float kvmean=0;
    if((samples-naec)>0){
        for(i=naec;i<samples;i++){
            printf("(PLS-%d): I[%f(mA), %d(RAW)]  V[%f(kV), %d(RAW)] \n",(int) (i-naec),((((float) is[i])*200.0)/255.0), is[i],pcb190ConvertKvRead(vs[i]),vs[i]);
            imean+=(float) is[i];
            vmean+=(float) vs[i];
        }
        imean=imean/(float)(samples-naec);
        vmean=vmean/(float)(samples-naec);
        scarto_v = 0;
        kvmean = pcb190ConvertKvRead(nearest(vmean));
        
        // Calcola scarto medio Impulso se ci sono campioni
        for(i=naec;i<samples;i++){
            scarto_v +=  ( ((kvmean - pcb190ConvertKvRead(vs[i])))* ((kvmean - pcb190ConvertKvRead(vs[i]))));
        }
        scarto_v = (10.0 * sqrt(scarto_v/(float)(samples-naec)));
    }


    // Legge il valore medio del tempo di pre impulso (se c'è stato)
    tmed_pre = 0;
    if(Ser422ReadRegister(_REGID(REG190_RX_TIME_PRE),10,&PCB190_CONTEST) == _SER422_NO_ERROR)
      tmed_pre = (unsigned short)((float) _DEVREG(REG190_RX_TIME_PRE,PCB190_CONTEST) * 1.115) ;

    // Legge il valore medio del tempo di impulso
    tmed_pls = 0;
    if(Ser422ReadRegister(_REGID(REG190_RX_TIME_PLS),10,&PCB190_CONTEST) == _SER422_NO_ERROR)
      tmed_pls = (unsigned short)((float) _DEVREG(REG190_RX_TIME_PLS,PCB190_CONTEST) * 1.115);
  

    // Lettura I FILAMENTO DURANTE RAGGI
    Ser422ReadRegister(_REGID(RG_SAMPLED_IFIL),10,&PCB190_CONTEST);
    ifil_rxend = _DEVREGL(RG_SAMPLED_IFIL,PCB190_CONTEST);
       

    // Legge i campioni di tempo se ci sono
    int sample_time = 0;


    // Stampa dei valori
    printf("HV-BUS(V):%f\n",(float) _DEVREGL(RG190_HV_RXEND,PCB190_CONTEST) * ((float) generalConfiguration.pcb190Cfg.HV_CONVERTION / 1000.0));
    printf("I_FIL (mA):%f\n",ifil_rxend * 47.98);
    
    if(naec){
      printf("AEC-Imean(mA)=%f\n",(aec_imean*200.)/255.);
      printf("AEC-kVmean(kV)=%f\n",(float) pcb190ConvertKvRead(nearest(aec_vmean)));
      printf("Tmed_Pre(ms)=%d\n",tmed_pre);
    }
    if(samples-naec){
      printf("PLS-I(mA)=%f\n",(imean*200.)/255.);
      printf("PLS-V(kV)=%f, dKv:%f\n",kvmean,scarto_v/10.);
      printf("Tmed_Pulse(ms)=%d\n",tmed_pls );
   }
    
    
    data[RX_END_CODE]=code;       // Risultato esposizione    
    data[MAS_PRE_L]=(unsigned char) (mAs_pre&0xFF);       
    data[MAS_PRE_H]=(unsigned char) ((mAs_pre>>8)&0xFF);  
    data[MAS_PLS_L]=(unsigned char) (mAs_erogati&0xFF);       
    data[MAS_PLS_H]=(unsigned char) ((mAs_erogati>>8)&0xFF);  
    data[I_MEAN]= (unsigned char) nearest(imean);     // Valore medio
    data[V_MEAN]= (unsigned char) nearest(vmean);     // Valore medio
    data[V_SIGMA] = (unsigned char) nearest(scarto_v);
    
    data[T_MEAN_PRE_L]=(unsigned char) (tmed_pre&0xFF);  
    data[T_MEAN_PRE_H]=(unsigned char) ((tmed_pre>>8)&0xFF);        
    data[T_MEAN_PLS_L]=(unsigned char) (tmed_pls&0xFF);  
    data[T_MEAN_PLS_H]=(unsigned char) ((tmed_pls>>8)&0xFF);   
    data[HV_POST_RAGGI] = _DEVREGL(RG190_HV_RXEND,PCB190_CONTEST);
    data[IFIL_POST_RAGGI] = ifil_rxend;
    
    // Carica i campionamenti dell'esposizione effettuata

    data[NSAMPLES_AEC] = (unsigned char) naec;
    if((samples - naec) >0) data[NSAMPLES_PLS] = (unsigned char) (samples - naec);
    else data[NSAMPLES_PLS] = 0;
  
    printf("AEC:%d, PLS:%d \n",data[NSAMPLES_AEC], data[NSAMPLES_PLS]);

    int index = SAMPLES_BUFFER;
    for(i=0; i<samples; i++, index++){
        if(index>=sizeof(data)) break;
        data[index] = is[i];
    }
   
    for(i=0; i<samples; i++, index++){
        if(index>=sizeof(data)) break;
        data[index] = vs[i];
    }
    
    // Aggiungo il numero di campionamenti di tempo
    data[index++] = sample_time;

    // Aggiungo i campionamenti di tempo
    for(i=0; i<sample_time; i++, index++){
        if(index>=sizeof(data)) break;
        data[index] = ts[i];
    }


    MCC_ENDPOINT ep = {_DEF_MCC_MASTER_TO_APP_MASTER};
    mccSendBuffer(&ep,MCC_RAGGI_DATA,0, data, index);
    
    
}    



/* EOF */
