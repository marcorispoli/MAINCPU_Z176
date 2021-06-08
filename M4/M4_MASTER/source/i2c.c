/**
 * @file i2c.c
 *
 * @brief Implementazione dell'interfaccia I2C.
 *
 * L'interfaccia I2C � realizzata tramite due pin GPIO del microprocessore.
 * Nel presente file sono opportunamente configurati i suddetti pin ed
 * � sviluppato l'intero protocollo I2C dal lato master.
 */


#define _I2C_C
#include "dbt_m4.h"
#include "i2c.h"

//#define I2C_DEBUG
#ifdef I2C_DEBUG
    #define PRINT printf
#else
    #define PRINT nop
    void nop(void* n, ...) {}
#endif

#define I2C_SCL_PIN  LWGPIO_PIN_PTB19
#define I2C_SCL_MUX  LWGPIO_MUX_PTB19_GPIO
#define I2C_SDA_PIN  LWGPIO_PIN_PTB21
#define I2C_SDA_MUX  LWGPIO_MUX_PTB21_GPIO
#define I2C_DELAY    100

static LWGPIO_STRUCT i2cSDA;
static LWGPIO_STRUCT i2cSCL;
static int i2cCounter = 0;


/**
 * @brief Configura i pin dell'intefaccia I2C.
 *
 * I segnali SCL e SDA sono configurati come open-collector a cui sono connessi opportuni pullup.
 * A tale scopo sono impostati permanentemente al livello logico basso e vengono alternativamente
 * settati come uscite o come ingressi se devono assumere rispettivamente valore basso oppure alto.
 */
void i2cInit()
{
    PRINT("i2cInit()\n");

    if (!lwgpio_init(&i2cSCL, I2C_SCL_PIN, LWGPIO_DIR_INPUT, LWGPIO_VALUE_LOW))
    {
        printf("Error: Failure in lwgpio_init");
        _mqx_exit(-1);
    }

    if (!lwgpio_init(&i2cSDA, I2C_SDA_PIN, LWGPIO_DIR_INPUT, LWGPIO_VALUE_LOW))
    {
        printf("Error: Failure in lwgpio_init");
        _mqx_exit(-1);
    }

    lwgpio_set_functionality(&i2cSCL, I2C_SCL_MUX);
    lwgpio_set_functionality(&i2cSDA, I2C_SDA_MUX);

    if (!lwgpio_set_attribute(&i2cSCL, LWGPIO_ATTR_DRIVE_STRENGTH, LWGPIO_AVAL_DRIVE_STRENGTH_HIGH))
    {
        printf("Error: Failure in lwgpio_set_attribute");
        _mqx_exit(-1);
    }

    if (!lwgpio_set_attribute(&i2cSDA, LWGPIO_ATTR_DRIVE_STRENGTH, LWGPIO_AVAL_DRIVE_STRENGTH_HIGH))
    {
        printf("Error: Failure in lwgpio_set_attribute");
        _mqx_exit(-1);
    }
}

/**
 * @brief Realizza un intervallo di ritardo base per regolare le temporizzazioni dei segnali.
 *
 * Il segnale di clock SCL � impostato al valore alto per un intervallo e al valore basso per
 * due intervalli. Tra il primo e il secondo intervallo della fase bassa, il segnale dati SDA
 * viene modificato al nuovo valore. Il periodo di clock � quindi composto da 3 volte l'intervallo
 * realizzato dalla presente funzione.
 *
 * La macro @c I2C_DELAY definisce la durata di tale intervallo. Un valore pari a 100 implica
 * una durata dell'intervallo pari a circa 5us. Il clock avr� quindi un periodo pari a 15us
 * cio� una frequenza pari a 66kHz.
 *
 * @warning La variabile del contatore @c i2cCounter deve essere esterna alla funzione
 * per evitare ottimizzazioni da parte del compilatore.
 */
void i2cDelay()
{
    i2cCounter = 0;
    while (i2cCounter < I2C_DELAY)
        i2cCounter++;
}

/**
 * @brief Esegue un ciclo di ripetizioni dell'intervallo realizzato da @c i2cDelay().
 *
 * @param loops Il numero di iterazioni da eseguire.
 */
void i2cLoopDelay(int loops)
{
    for (int i = 0; i < loops; i++)
        i2cDelay();
}

/**
 * @brief Assegna lo stato del segnale SCL.
 *
 * @param l Il livello logico da assegnare al segnale SCL.
 */
void i2cSetSCL(int l)
{
    lwgpio_set_direction(&i2cSCL, (l ? LWGPIO_DIR_INPUT : LWGPIO_DIR_OUTPUT));
}

/**
 * @brief Restituisce lo stato del segnale SCL.
 *
 * @return Il livello logico del segnale SCL.
 */
int i2cGetSCL()
{
    return (int)lwgpio_get_value(&i2cSCL);
}

/**
 * @brief Assegna lo stato del segnale SDA.
 *
 * @param l Il livello logico da assegnare al segnale SDA.
 */
void i2cSetSDA(int l)
{
    lwgpio_set_direction(&i2cSDA, (l ? LWGPIO_DIR_INPUT : LWGPIO_DIR_OUTPUT));
}

/**
 * @brief Restituisce lo stato del segnale SDA.
 *
 * @return Il livello logico del segnale SDA.
 */
int i2cGetSDA()
{
    return (int)lwgpio_get_value(&i2cSDA);
}

/**
 * @brief Realizza la sequenza di start in apertura di trasmissione.
 */
void i2cStart()
{
    i2cDelay();
    i2cSetSDA(0);
    i2cDelay();
    i2cSetSCL(0);
    i2cDelay();
}

/**
 * @brief Realizza la sequenza di stop in chiusura di trasmissione.
 */
void i2cStop()
{
    i2cDelay();
    i2cSetSCL(1);
    i2cDelay();
    i2cSetSDA(1);
    i2cDelay();
}

/**
 * @brief Attende il rilascio del segnale SCL da parte dello slave.
 *
 * Il segnale SCL � implementato come open-collector per cui pu� essere utilizzato
 * dallo slave coinvolto nella comunicazione, per segnalare, mantenendolo basso,
 * la necessit� che si attenda che abbia completato le sue operazioni interne
 * prima di riprendere la comunicazione.
 *
 * @warning Occorre implementare il controllo che il segnale non resti bloccato
 * permanentemente.
 */
void i2cWaitForSCL()
{
    while (i2cGetSCL() == 0)
        i2cDelay();
}

/**
 * @brief Esegue l'invio di un byte.
 *
 * Il pacchetto di 8 bit � seguito da un ulteriore ciclo per la lettura del bit di
 * acknowledge restituito dallo slave.
 *
 * @param data Il byte da inviare.
 *
 * @return Lo stato di acknowledge restituito dallo slave.
 *
 * @note L'attesa del rilascio del segnale SCL da parte dello slave � al momento sospesa,
 * in attesa che venga implementato il controllo che eviti il blocco permanente.
 *
 * @see i2cWaitForSCL
 */
bool i2cSendPacket(unsigned char data)
{
    for (int i = 7; i >= 0; i--)
    {
        i2cSetSDA(data & (1 << i));
        i2cDelay();
        i2cSetSCL(1);
        i2cDelay();
//        i2cWaitForSCL();
        i2cSetSCL(0);
        i2cDelay();
    }

    // Acknowledge bit
    i2cSetSDA(1);
    i2cDelay();
    i2cSetSCL(1);
    i2cDelay();
    int sda_9th = i2cGetSDA();
    i2cSetSCL(0);
    i2cDelay();

    // End of packet
    i2cSetSDA(0);

    return (sda_9th == 0);
}

/**
 * @brief Esegue l'invio del pacchetto di indirizzo dello slave.
 *
 * @param addr Indirizzo a 7 bit dello slave interessato dalla trasmissione.
 *
 * @param read Indica se si intende eseguire una lettura, altrimenti si eseguir�
 * una scrittura.
 *
 * @return Lo stato di acknowledge restituito dallo slave.
 */
bool i2cSendAddress(unsigned char addr, bool read)
{
    unsigned char data;

    data  = (addr << 1);
    data |= (read ? 1 : 0);
    return i2cSendPacket(data);
}

/**
 * @brief Esegue la ricezione di un byte.
 *
 * Al termine della ricezione degli 8 bit di dato, nel 9� ciclo di clock, si risponder�
 * con lo stato di acknowledge specificato.
 *
 * @param ack Stato di acknowledge da rispondere al termine della ricezione.
 *
 * @return Il byte ricevuto.
 */
unsigned char i2cRecvPacket(bool ack)
{
    unsigned char data = 0;

    // Release data line
    i2cSetSDA(1);

    for (int i = 7; i >= 0; i--)
    {
        i2cDelay();
        i2cSetSCL(1);
        i2cDelay();
        data |= (i2cGetSDA() ? (1 << i) : 0);
        i2cSetSCL(0);
        i2cDelay();
    }

    // Acknowledge bit
    i2cSetSDA(ack ? 0 : 1);
    i2cDelay();
    i2cSetSCL(1);
    i2cDelay();
    i2cSetSCL(0);
    i2cDelay();

    // End of packet
    i2cSetSDA(0);

    return data;
}

/**
 * @brief Esegue l'invio di un buffer di byte.
 *
 * @param addr Indirizzo dello slave interessato dalla trasmissione.
 *
 * @param reg Indirizzo del registro dello slave destinazione della trasmissione.
 * In caso di auto-incremento del puntatore dei registri, rappresenta il registro iniziale.
 *
 * @param data Indirizzo del buffer di byte da inviare.
 *
 * @param size Dimensione del buffer di byte da inviare.
 *
 * @return Lo stato di acknowledge restituito dallo slave.
 */
bool i2cSendMessage(unsigned char addr, unsigned char reg, unsigned char* data, int size)
{
    bool acknowledged = true;

    i2cStart();

    if (!i2cSendAddress(addr, false))
    {
        acknowledged = false;
        goto finish;
    }

    if (!i2cSendPacket(reg))
    {
        acknowledged = false;
        goto finish;
    }

    for (int i = 0; i < size; i++)
    {
        if (!i2cSendPacket(data[i]))
        {
            acknowledged = false;
            goto finish;
        }
    }

finish:
    i2cStop();

    return acknowledged;
}

/**
 * @brief Esegue la ricezione di un buffer di byte.
 *
 * @param addr Indirizzo dello slave interessato dalla trasmissione.
 *
 * @param reg Indirizzo del registro dello slave sorgente della trasmissione.
 * In caso di auto-incremento del puntatore dei registri, rappresenta il registro iniziale.
 *
 * @param data Indirizzo del buffer in cui copiare i byte ricevuti.
 *
 * @param size Dimensione del buffer di byte da ricevere.
 *
 * @return Lo stato di acknowledge restituito dallo slave.
 */
bool i2cRecvMessage(unsigned char addr, unsigned char reg, unsigned char* data, int size)
{
    bool acknowledged = true;

    i2cStart();

    if (!i2cSendAddress(addr, false))
    {
        acknowledged = false;
        goto finish;
    }

    if (!i2cSendPacket(reg))
    {
        acknowledged = false;
        goto finish;
    }

    i2cStop();

    i2cStart();

    if (!i2cSendAddress(addr, true))
    {
        acknowledged = false;
        goto finish;
    }

    for (int i = 0; i < size-1; i++)
        data[i] = i2cRecvPacket(true);

    data[size-1] = i2cRecvPacket(false);

finish:
    i2cStop();

    return acknowledged;
}

/**
 * @brief Esegue la ricerca di un dispositivo sul bus.
 *
 * @param addr Indirizzo del dispositivo da cercare.
 *
 * @return Lo stato di acknowledge ricevuto e quindi l'esito della ricerca.
 */
bool i2cPing(unsigned char addr)
{
    PRINT("i2cPing(0x%02x)\n", addr);

    bool acknowledged = true;

    i2cStart();

    acknowledged = i2cSendAddress(addr, false);

    i2cStop();

    return acknowledged;
}

/* EOF */
