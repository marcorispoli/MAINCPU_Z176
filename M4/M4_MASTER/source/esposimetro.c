#define _ESPOSIMETRO_C
#include "dbt_m4.h" 

// Restituisce il valore PLOG letto dall'esposimetro durante il pre-impulso
unsigned int getPlog(unsigned short rad){
    if(rad>sizeof(rToPlog)) return 160;
    return (int) rToPlog[rad];
}

unsigned int getPrad(unsigned short rad){
    float p = (float) getPlog(rad) * 1.74;
    unsigned int pi = (unsigned int) p;
    if((p -(float) pi) > 0.5) return pi+1;
    else return pi;

}

// ______________________________________________________________________________________________

/* EOF */
   


