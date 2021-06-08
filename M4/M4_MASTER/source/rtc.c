/**
 * @file rtc.c
 *
 * @brief Implementa la gestione del modulo RTC del sistema.
 *
 * Nel sistema la funzionalità RTC è realizzata per il tramite del modulo
 * RV-3028-C7 oppure RV-4162-C7, entrambi forniti da Micro Crystal Switzerland.
 *
 * Il codice contenuto nel presente file permette di rilevare quale modulo sia installato
 * nel sistema e provvede ad interfacciarlo opportunamente allo scopo di realizzare la
 * funzione di orologio e calendario.
 *
 * Le informazioni di ora e data generate dai due moduli sono leggermente differenti.
 * Il modulo RV-3028-C7, al contrario del modulo RV-4162-C7, non restituisce i centesimi
 * di secondo. Inoltre il campo dell'anno si estende sull'arco di un secolo, dal 2000 al
 * 2099, nel caso del RV-3028-C7, mentre si estende su 4 secoli, dal 2000 al 2399, nel
 * caso del RV-4162-C7.
 *
 * Gli indirizzi dei registri che contengono le informazioni di data e ora sono
 * organizzati diversamente e il codice implementato gestisce tali differenze restituendo
 * valori opportunamente normalizzati.
 */


#define _RTC_C
#include "dbt_m4.h"
#include "rtc.h"
#include "i2c.h"
#include <ctype.h>
#include <stdlib.h>


//#define RTC_DEBUG
#ifdef RTC_DEBUG
    #define PRINT printf
#else
    #define PRINT nop
    void nop(void* n, ...) {}
#endif

#define RTC_ADDR_RV3028C7 0x52
#define RTC_ADDR_RV4162C7 0x68




typedef enum Rv3028Enum {
    BSM_SWITCHOVER_DISABLED = 0,
    BSM_DIRECT_SWITCHING_MODE,
    BSM_SWITCHOVER_DISABLED2,
    BSM_LEVEL_SWITCHING_MODE
} Rv3028;


static bool rtcInitialized = false;
static bool rtcDataAvailable = false;
static unsigned char rtcAddress = 0;
static unsigned char rtcClockData[8];

static char* rtcWeekdays[] = {"XXX", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

static char rtcDate[32];


/**
 * @brief Inizializza la funzionalità di RTC del sistema.
 *
 * Se l'inizializzazione è già stata eseguita non viene ripetuta.
 *
 * L'interfaccia I2C viene inizializzata e i possibili moduli RTC vengono cercati
 * sul bus I2C.
 *
 * Se viene trovato il modulo RV-3028-C7, dotato di alimentazione di backup,
 * vengono abilitate le relative funzioni di "Backup Switchover", in modalità direct,
 * e di "Trickle Charger", per la carica della capacità di backup.
 *
 * @return L'esito dell'operazione. 0 per il successo, -1 per il fallimento.
 */
int rtcInit()
{
    PRINT("rtcInit())\n");

    if (rtcInitialized)
        return 0;

    i2cInit();

    if (i2cPing(RTC_ADDR_RV3028C7))
    {
        rtcAddress = RTC_ADDR_RV3028C7;
        PRINT("RV3028C7 found\n");
    }
    else if (i2cPing(RTC_ADDR_RV4162C7))
    {
        rtcAddress = RTC_ADDR_RV4162C7;
        PRINT("RV4162C7 found\n");
    }
    else
    {
        printf("Error: RTC not found\n");
        return -1;
    }

    if (rtcAddress == RTC_ADDR_RV3028C7)
    {
        unsigned char reg  = 0x37; // EEPROM Backup register
        unsigned char data = 0, newdata = 0;

        if (!i2cRecvMessage(rtcAddress, reg, &data, 1))
        {
            printf("Error: NACK received\n");
            return -1;
        }

        newdata = data;

        int bsm = (data & 0x0c) >> 2; // Backup Switchover Mode
        int tce = (data & 0x20) >> 5; // Trickle Charger Enable

        if (bsm != BSM_DIRECT_SWITCHING_MODE)
        {
            PRINT("Enabling Switchover\n");

            newdata &= 0xf3;
            newdata |= (BSM_DIRECT_SWITCHING_MODE << 2);
        }

        if (!tce)
        {
            PRINT("Enabling Trickle Charger\n");

            newdata &= 0xdf;
            newdata |= (1 << 5);
        }

        if (newdata != data)
        {
            if (!i2cSendMessage(rtcAddress, reg, &newdata, 1))
            {
                printf("Error: NACK received\n");
                return -1;
            }
        }
    }

    for (int i = 0; i < sizeof(rtcClockData); rtcClockData[i++] = 0);

    rtcInitialized = true;
    rtcDataAvailable = false;

    return 0;
}

/**
 * @brief Esegue la lettura dei registri dei contatori di orologio e calendario.
 *
 * Da entrambi i moduli vengono scaricati solo i primi 8 registri che contengono
 * tutti i contatori necessari per tracciare il tempo.
 *
 * I dati di data e ora sono quindi disponibili nel buffer @c rtcClockData
 * per essere letti tramite la funzione @c rtcGet() e modificati tramite la funzione
 * @c rtcSet().
 *
 * @return L'esito dell'operazione. 0 per il successo, -1 per il fallimento.
 */
int rtcRecvClockData()
{
    PRINT("rtcRecvClockData()\n");

    if (!rtcInitialized)
    {
        printf("Error: RTC not initialized\n");
        return -1;
    }

    if (!i2cRecvMessage(rtcAddress, 0, rtcClockData, sizeof(rtcClockData)))
    {
        printf("Error: NACK received\n");
        return -1;
    }

    rtcDataAvailable = true;

    return 0;
}

/**
 * @brief Esegue la scrittura dei registri dei contatori di orologio e calendario.
 *
 * I dati devono essere stati precedentemente letti e copiati nel buffer @c rtcClockData
 * ed eventualmente modificati tramite la funzione @c rtcSet().
 *
 * @return L'esito dell'operazione. 0 per il successo, -1 per il fallimento.
 */
int rtcSendClockData()
{
    PRINT("rtcSendClockData()\n");

    if (!rtcInitialized)
    {
        printf("Error: RTC not initialized\n");
        return -1;
    }

    if (!rtcDataAvailable)
    {
        printf("Error: No data available\n");
        return -1;
    }

    if (!i2cSendMessage(rtcAddress, 0, rtcClockData, sizeof(rtcClockData)))
    {
        printf("Error: NACK received\n");
        return -1;
    }

    return 0;
}

/**
 * @brief Esegue la stampa dei registri dei contatori scaricati dall'RTC.
 */
void rtcLogClockData()
{
    PRINT("rtcLogClockData()\n");

    int lines = (sizeof(rtcClockData) + 7) / 8;
    for (int i = 0; i < lines; i++)
    {
        printf("  ");
        for (int j = 0; j < 8; j++)
        {
            printf("%02x ", rtcClockData[8*i+j]);
        }
        printf("\n");
    }
}

/**
 * @brief Restituisce il valore di un campo dei dati di orologio per il modulo RV-3028-C7.
 *
 * @param field  Campo da restituire.
 *
 * @return Valore del campo richiesto.
 */
int rtcGet_RV3028C7(RtcField field)
{
    int value = 0;

    switch (field)
    {
    case RTC_HUNDREDS:
        value = 0;
        break;

    case RTC_SECONDS:
        value = 10 * ((rtcClockData[0] & 0x70) >> 4) + (rtcClockData[0] & 0x0f);
        break;

    case RTC_MINUTES:
        value = 10 * ((rtcClockData[1] & 0x70) >> 4) + (rtcClockData[1] & 0x0f);
        break;

    case RTC_HOURS:
        value = 10 * ((rtcClockData[2] & 0x30) >> 4) + (rtcClockData[2] & 0x0f);
        break;

    case RTC_DAY:
        value = 10 * ((rtcClockData[4] & 0x30) >> 4) + (rtcClockData[4] & 0x0f);
        break;

    case RTC_MONTH:
        value = 10 * ((rtcClockData[5] & 0x10) >> 4) + (rtcClockData[5] & 0x0f);
        break;

    case RTC_YEAR:
        value = 2000 + 10 * ((rtcClockData[6] & 0xf0) >> 4) + (rtcClockData[6] & 0x0f);
        break;

    case RTC_WEEKDAY:
        value = rtcClockData[3] & 0x07;
        value++;
        break;
    }

    return value;
}

/**
 * @brief Restituisce il valore di un campo dei dati di orologio per il modulo RV-4162-C7.
 *
 * @param field  Campo da restituire.
 *
 * @return Valore del campo richiesto.
 */
int rtcGet_RV4162C7(RtcField field)
{
    int value = 0;

    switch (field)
    {
    case RTC_HUNDREDS:
        value = 10 * ((rtcClockData[0] & 0xf0) >> 4) + (rtcClockData[0] & 0x0f);
        break;

    case RTC_SECONDS:
        value = 10 * ((rtcClockData[1] & 0x70) >> 4) + (rtcClockData[1] & 0x0f);
        break;

    case RTC_MINUTES:
        value = 10 * ((rtcClockData[2] & 0x70) >> 4) + (rtcClockData[2] & 0x0f);
        break;

    case RTC_HOURS:
        value = 10 * ((rtcClockData[3] & 0x30) >> 4) + (rtcClockData[3] & 0x0f);
        break;

    case RTC_DAY:
        value = 10 * ((rtcClockData[5] & 0x30) >> 4) + (rtcClockData[5] & 0x0f);
        break;

    case RTC_MONTH:
        value = 10 * ((rtcClockData[6] & 0x10) >> 4) + (rtcClockData[6] & 0x0f);
        break;

    case RTC_YEAR:
        value  = 2000 + 100 * ((rtcClockData[6] & 0xc0) >> 6);
        value += 10 * ((rtcClockData[7] & 0xf0) >> 4) + (rtcClockData[7] & 0x0f);
        break;

    case RTC_WEEKDAY:
        value = rtcClockData[4] & 0x07;
        break;
    }

    return value;
}

/**
 * @brief Restituisce il valore di un campo dei dati di orologio e calendario.
 *
 * Il modulo RV-3028-C7 non possiede l'informazione relativa ai centesimi di secondo.
 * Viene quindi restituito il valore nullo.
 *
 * Per il modulo RV-3028-C7 il valore dell'anno è compreso tra 2000 e 2099,
 * mentre per il modulo RV-4162-C7 il valore è compreso tra 2000 e 2399.
 *
 * @param field  Campo da restituire.
 *
 * @return Valore del campo richiesto.
 */
int rtcGet(RtcField field)
{
    if (rtcAddress == RTC_ADDR_RV3028C7)
    {
        return rtcGet_RV3028C7(field);
    }
    else if (rtcAddress == RTC_ADDR_RV4162C7)
    {
        return rtcGet_RV4162C7(field);
    }
    else
    {
        return -1;
    }
}

/**
 * @brief Imposta il valore di un campo dei dati di orologio per il modulo RV-3028-C7.
 *
 * @param field  Campo da modificare.
 * @param value  Valore da impostare.
 */
void rtcSet_RV3028C7(RtcField field, int value)
{
    unsigned char temp;

    switch (field)
    {
    case RTC_HUNDREDS:
        break;

    case RTC_SECONDS:
        if (value < 0 || value > 59)
            break;
        temp  = value % 10;
        temp += (value / 10) << 4;
        temp &= 0x7f;
        rtcClockData[0] &= 0x80;
        rtcClockData[0] |= temp;
        break;

    case RTC_MINUTES:
        if (value < 0 || value > 59)
            break;
        temp  = value % 10;
        temp += (value / 10) << 4;
        temp &= 0x7f;
        rtcClockData[1] &= 0x80;
        rtcClockData[1] |= temp;
        break;

    case RTC_HOURS:
        if (value < 0 || value > 23)
            break;
        temp  = value % 10;
        temp += (value / 10) << 4;
        temp &= 0x3f;
        rtcClockData[2] &= 0xc0;
        rtcClockData[2] |= temp;
        break;

    case RTC_DAY:
        if (value < 1 || value > 31)
            break;
        temp  = value % 10;
        temp += (value / 10) << 4;
        temp &= 0x3f;
        rtcClockData[4] &= 0xc0;
        rtcClockData[4] |= temp;
        break;

    case RTC_MONTH:
        if (value < 1 || value > 12)
            break;
        temp  = value % 10;
        temp += (value / 10) << 4;
        temp &= 0x1f;
        rtcClockData[5] &= 0xe0;
        rtcClockData[5] |= temp;
        break;

    case RTC_YEAR:
        if (value < 2000 || value > 2099)
            break;
        value  %= 100;
        temp  = value % 10;
        temp += (value / 10) << 4;
        temp &= 0x00ff;
        rtcClockData[6] = temp;
        break;

    case RTC_WEEKDAY:
        if (value < 1 || value > 7)
            break;
        value--;
        temp = value & 0x07;
        rtcClockData[3] &= 0xf8;
        rtcClockData[3] |= temp;
        break;
    }
}

/**
 * @brief Imposta il valore di un campo dei dati di orologio per il modulo RV-4162-C7.
 *
 * @param field  Campo da modificare.
 * @param value  Valore da impostare.
 */
void rtcSet_RV4162C7(RtcField field, int value)
{
    unsigned char temp;

    switch (field)
    {
    case RTC_HUNDREDS:
        rtcClockData[0] = 0;
        break;

    case RTC_SECONDS:
        if (value < 0 || value > 59)
            break;
        temp  = value % 10;
        temp += (value / 10) << 4;
        temp &= 0x7f;
        rtcClockData[1] &= 0x80;
        rtcClockData[1] |= temp;
        break;

    case RTC_MINUTES:
        if (value < 0 || value > 59)
            break;
        temp  = value % 10;
        temp += (value / 10) << 4;
        temp &= 0x7f;
        rtcClockData[2] &= 0x80;
        rtcClockData[2] |= temp;
        break;

    case RTC_HOURS:
        if (value < 0 || value > 23)
            break;
        temp  = value % 10;
        temp += (value / 10) << 4;
        temp &= 0x3f;
        rtcClockData[3] &= 0xc0;
        rtcClockData[3] |= temp;
        break;

    case RTC_DAY:
        if (value < 1 || value > 31)
            break;
        temp  = value % 10;
        temp += (value / 10) << 4;
        temp &= 0x3f;
        rtcClockData[5] &= 0xc0;
        rtcClockData[5] |= temp;
        break;

    case RTC_MONTH:
        if (value < 1 || value > 12)
            break;
        temp  = value % 10;
        temp += (value / 10) << 4;
        temp &= 0x1f;
        rtcClockData[6] &= 0xe0;
        rtcClockData[6] |= temp;
        break;

    case RTC_YEAR:
        if (value < 2000 || value > 2399)
            break;
        temp = ((value - 2000) / 100) << 6;
        temp &= 0xc0;
        rtcClockData[6] &= 0x3f;
        rtcClockData[6] |= temp;
        value  %= 100;
        temp  = value % 10;
        temp += (value / 10) << 4;
        temp &= 0x00ff;
        rtcClockData[7] = temp;
        break;

    case RTC_WEEKDAY:
        if (value < 1 || value > 7)
            break;
        temp = value & 0x07;
        rtcClockData[4] &= 0xf8;
        rtcClockData[4] |= temp;
        break;
    }
}

/**
 * @brief Imposta il valore di un campo dei dati di orologio e calendario.
 *
 * Il modulo RV-3028-C7 non possiede l'informazione relativa ai centesimi di secondo.
 * Viene quindi ignorato.
 *
 * Per il modulo RV-3028-C7 il valore dell'anno è compreso tra 2000 e 2099,
 * mentre per il modulo RV-4162-C7 il valore è compreso tra 2000 e 2399.
 *
 * @param field  Campo da modificare.
 * @param value  Valore da impostare.
 */
void rtcSet(RtcField field, int value)
{
    if (rtcAddress == RTC_ADDR_RV3028C7)
    {
        rtcSet_RV3028C7(field, value);
    }
    else if (rtcAddress == RTC_ADDR_RV4162C7)
    {
        rtcSet_RV4162C7(field, value);
    }
}

/**
 * @brief Restituisce l'informazione di data e orario corrente dall'RTC.
 *
 * I registri dei contatori RTC vengono letti dal modulo e l'informazione
 * di data e orario viene scritta sul buffer interno @c rtcDate nel formato:
 * @code
 *   Mon 2020-11-30 17:34:15
 * @endcode
 *
 * @return Puntatore al buffer di caratteri interno contenente l'informazione.
 */
char* rtcGetDate()
{
    PRINT("rtcGetDate()\n");

    if (rtcRecvClockData() < 0)
        return NULL;

    sprintf(rtcDate, "%s %04d-%02d-%02d %02d:%02d:%02d",
            rtcWeekdays[rtcGet(RTC_WEEKDAY)],
            rtcGet(RTC_YEAR),
            rtcGet(RTC_MONTH),
            rtcGet(RTC_DAY),
            rtcGet(RTC_HOURS),
            rtcGet(RTC_MINUTES),
            rtcGet(RTC_SECONDS));

    return rtcDate;
}

/**
 * @brief Esegue il parsing della stringa contenente data e ora e restituisce i campi separati.
 *
 * La stringa @c date, passata dalla funzione @c rtcSetDate() descrive l'informazione di data
 * e ora da impostare sull'RTC. Il formato di tale stringa è del tipo:
 * @code
 *   Mon 2020-11-30 17:34:15
 * @endcode
 *
 * I 7 campi devono essere tutti presenti, nell'ordine mostrato, ma possono essere separati
 * da uno o più caratteri scelti liberamente dall'elenco:
 * - spazio
 * - tabulazione
 * - meno
 * - più
 * - asterisco
 * - slash
 * - due punti
 * - punto e virgola
 * - punto
 * - virgola
 * - underscore
 *
 * Il campo del giorno della settimana è composto da almeno 3 caratteri, non importa il case, e
 * deve essere compreso nell'elenco dove è indicato anche il valore restituito:
 * - 'sun' (= 1)
 * - 'mon' (= 2)
 * - 'tue' (= 3)
 * - 'wed' (= 4)
 * - 'thu' (= 5)
 * - 'fri' (= 6)
 * - 'sat' (= 7)
 *
 * Seguono esempi di stringhe valide:
 * @code
 *   mOn  2020 11  30 17   34 15
 *   MON.2020.11.30.17.34.15
 *   mondXy.. 2020 - 11 / 30 _ 17+*34*-15
 * @endcode
 *
 * @param date     Il buffer contenente la stringa da interpretare.
 * @param weekday  L'indirizzo del campo del giorno della settimana (da 1 a 7)
 * @param year     L'indirizzo del campo dell'anno (da 2000 a 2099 oppure da 2000 a 2399)
 * @param month    L'indirizzo del campo del mese (da 1 a 12)
 * @param day      L'indirizzo del campo del giorno del mese (da 1 a 31)
 * @param hours    L'indirizzo del campo delle ore (da 0 a 23)
 * @param minutes  L'indirizzo del campo dei minuti (da 0 a 59)
 * @param seconds  L'indirizzo del campo dei secondi (da 0 a 59)
 *
 * @return L'esito dell'operazione. 0 per il successo, -1 per il fallimento.
 */
int rtcParseDate(char* date, int* weekday, int* year, int* month, int* day, int* hours, int* minutes, int* seconds)
{
    PRINT("rtcParseDate(\"%s\")\n", date);

    char* token = NULL;
    char* delim = " \t-+*/:;.,_";
    char  copy[128];

    // La funzione strtok altera il buffer di lavoro. Meglio farne una copia.
    strncpy(copy, date, 127);
    copy[127] = 0;


    // Si può usare codice rientrante (__USE_POSIX)????
    //    char* saveptr;
    //    char* token = strtok_r(clock, " -:.", &saveptr);


    // weekday
    if ((token = strtok(copy, delim)) == NULL)
    {
        printf("Error: Invalid date format");
        return -1;
    }

    *weekday = 0;
    for (int i = 1; i < 8; i++)
    {
        bool found = true;
        for (int j = 0; j < 3; j++)
        {
            if (tolower(token[j]) != tolower(rtcWeekdays[i][j]))
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            *weekday = i;
            break;
        }
    }

    if (*weekday == 0)
    {
        printf("Error: Invalid date format");
        return -1;
    }


    // year
    if ((token = strtok(NULL, delim)) == NULL)
    {
        printf("Error: Invalid date format");
        return -1;
    }

    *year = atoi(token);


    // month
    if ((token = strtok(NULL, delim)) == NULL)
    {
        printf("Error: Invalid date format");
        return -1;
    }

    *month = atoi(token);


    // day
    if ((token = strtok(NULL, delim)) == NULL)
    {
        printf("Error: Invalid date format");
        return -1;
    }

    *day = atoi(token);


    // hours
    if ((token = strtok(NULL, delim)) == NULL)
    {
        printf("Error: Invalid date format");
        return -1;
    }

    *hours = atoi(token);


    // minutes
    if ((token = strtok(NULL, delim)) == NULL)
    {
        printf("Error: Invalid date format");
        return -1;
    }

    *minutes = atoi(token);


    // seconds
    if ((token = strtok(NULL, delim)) == NULL)
    {
        printf("Error: Invalid date format");
        return -1;
    }

    *seconds = atoi(token);


    return 0;
}

/**
 * @brief Modifica l'informazione di data e orario corrente sull'RTC.
 *
 * L'informazione di data e ora desiderata viene fornita tramite stringa.
 * La stringa viene interpretata e scomposta nei diversi campi che vengono quindi
 * impostati nel buffer @c rtcClockData successivamente inviato all'RTC.
 *
 * Vedere la funzione @c rtcParse() per i dettagli del formato della stringa di ingresso
 * e i valori validi dei singoli campi.
 *
 * @param date Il buffer contenente la stringa da interpretare.
 *
 * @return L'esito dell'operazione. 0 per il successo, -1 per il fallimento.
 */
int rtcSetDate(char* date)
{
    PRINT("rtcSetDate(\"%s\")\n", date);

    if (!rtcDataAvailable)
    {
        if (rtcRecvClockData() < 0)
            return -1;
    }

    int weekday;
    int year;
    int month;
    int day;
    int hours;
    int minutes;
    int seconds;

    if (rtcParseDate(date, &weekday, &year, &month, &day, &hours, &minutes, &seconds) < 0)
        return -1;

    rtcSet(RTC_WEEKDAY, weekday);
    rtcSet(RTC_YEAR,    year);
    rtcSet(RTC_MONTH,   month);
    rtcSet(RTC_DAY,     day);
    rtcSet(RTC_HOURS,   hours);
    rtcSet(RTC_MINUTES, minutes);
    rtcSet(RTC_SECONDS, seconds);

    if (rtcSendClockData() < 0)
        return -1;

    return 0;
}

char* getWeekdayStr(unsigned char val){
    if(val>7) val=0;
    return rtcWeekdays[val];
}

/* EOF */
