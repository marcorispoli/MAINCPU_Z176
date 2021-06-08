#ifndef DOSE_H
#define DOSE_H

#include "application.h"


class DOSE
{


public:
    explicit DOSE(QString tubedir);

    #define _MIN_CGS_MM 20
    #define _MAX_CGS_MM 80
    #define _D_CGS_MM   5

    #define _MIN_CGS_DKV 200
    #define _MAX_CGS_DKV 350
    #define _D_CGS_DKV   5

    #define _LEN_CGS_MM (1+ ((_MAX_CGS_MM - _MIN_CGS_MM)/_D_CGS_MM))
    #define _LEN_CGS_KV (1+ ((_MAX_CGS_DKV - _MIN_CGS_DKV) / (_D_CGS_DKV)))

    typedef struct {
        // Parametri acquisiti dal Tubo
        //float kerma[_LEN_CGS_KV];
        bool  configured;
        int   D0; // Distanza dal piano del potter della misura del KERMA
        int   D1; // Distanza del fuoco dal piano del potter

        int   KVH; // kV di calibrazione alti
        int   KVM; // kV di calibrazione alti
        int   KVL; // kV di calibrazione alti
        float AKH;
        float AKM;
        float AKL;

        float cgs[1 + _LEN_CGS_KV][1 + _LEN_CGS_MM];
    }kermaCnf_Str;

    typedef struct {
        kermaCnf_Str MoKerma;
        kermaCnf_Str RhKerma;
    }doseCnf_Str;


    // Restituisce il puntatore alla struttura associata al filtro selezionato
    kermaCnf_Str* getKermaPtr(int filtro);
    kermaCnf_Str* getKermaPtr(QString filtro);
    QString getDoseUgDebug(int mm, int offset_mm, int dmAs, int dkv, int filtro);
public:
    QString tubeDir;
    bool kerma_mo_configured ;         // Airkerma Molibdeno configurato
    bool kerma_rh_configured ;         // Airkerma Rh configurato

    // Calcolatore di dose
    doseCnf_Str dose;

    bool readCgsConfig(kermaCnf_Str* pKerma, QString filename);
    float getKerma(kermaCnf_Str *pKerma, int dkV);
    bool readKermaConfig(QString filtro);
    bool storeKermaConfig(QString filtro);
    bool storeKermaConfig(QString filtro, int D0, int D1, int KVH, float AKH, int KVM, float AKM, int KVL, float AKL);
    float getDoseUg(int mm, int offset_mm, int dmAs, int dkv, int filtro);
    float getConvertedUgKerma(int dkv, int dmas, int mm, int filtro);

};

#endif // DOSE_H
