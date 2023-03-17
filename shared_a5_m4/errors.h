#ifndef _SHARED_ERRORS_H
#define _SHARED_ERRORS_H
  
#define SLAVE_ERROR_NONE            0
#define SLAVE_ERROR_LIFT_PEDALS     1 // Pedali Lift bloccati


// Errori dallo SLAVE verso GUI slave
#define PCB240_ERROR_TIMEOUT 1

// TO BE DONE: associare i codici di errore agli allarmi
typedef enum{
    // Errori implementati nel driver
    TRX_NO_ERRORS=0,
    TRX_ERROR_INVALID_STATUS,           // Comando richiesto in condizioni non consentite
    TRX_ERROR_ZERO_SETTING_INIT ,       // Error in preparing the zero setting sequence
    TRX_ERROR_ZERO_SETTING_EXECUTION,   // Error in the zero seqeunce execution
    TRX_ERROR_ZERO_SETTING_TMO,         // Timeout command execution
    TRX_ERROR_POSITION_SETTING_INIT,    // Init of the positioning command
    TRX_ERROR_POSITION_EXECUTION,
    TRX_ERROR_POSITION_TMO,
    TRX_OBSTACLE_ERROR,
    TRX_OBSTACLE_BLOCKED_ERROR,
    TRX_TIMEOUT_ERROR,
    TRX_OBSTACLE_OBSTRUCTION_ERROR,
    TRX_DEVICE_ERROR,                   // Errore dovuto ad un fault del device
    TRX_SAFETY_FAULT,                   // Errore segnalato dal dispositivo di sicurezza esterno

    // Errori gestiti dall'interfaccia
    TRX_DISABLED_ERROR,
    TRX_CAN_ERROR,
    TRX_RANGE_ERROR,
    TRX_BUSY,
    TRX_ERROR_UNDEFINED                 // Generic error

}_trx_errors_t;

typedef enum{
    ARM_NO_ERRORS=0,
    ARM_ERROR_INVALID_STATUS,           // Comando richiesto in condizioni non consentite
    ARM_ERROR_ZERO_SETTING_INIT ,       // Error in preparing the zero setting sequence
    ARM_ERROR_ZERO_SETTING_EXECUTION,   // Error in the zero seqeunce execution
    ARM_ERROR_ZERO_SETTING_TMO,         // Timeout command execution
    ARM_ERROR_POSITION_SETTING_INIT,    // Init of the positioning command
    ARM_ERROR_POSITION_EXECUTION,
    ARM_ERROR_POSITION_TMO,
    ARM_OBSTACLE_ERROR,
    ARM_OBSTACLE_BLOCKED_ERROR,
    ARM_OBSTACLE_OBSTRUCTION_ERROR,
    ARM_DEVICE_ERROR,                   // Errore dovuto ad un fault del device

    // Errori gestiti dall'interfaccia
    ARM_DISABLED_ERROR,
    ARM_CAN_ERROR,
    ARM_RANGE_ERROR,
    ARM_BUSY,
    ARM_SECURITY_SWITCHES,
    ARM_POWER_SUPPLY,
    ARM_ERROR_UNDEFINED                 // Generic error

}_arm_errors_t;

typedef enum{
    LENZE_NO_ERRORS=0,
    LENZE_ANALOG_CONNECTION_ERROR,
    LENZE_ANALOG_INPUT_ERROR,
    LENZE_DROP_ARM,
    LENZE_DEVICE_ERROR,

    LENZE_LOWER_ERROR,
    LENZE_UPPER_ERROR,
    LENZE_BUSY,
    LENZE_ERROR_UNDEFINED

}_lenze_errors_t;

//////////////////////////////////////////////////////////////////////////////
//              CODICI DI ERRORE RELATIVI ALLE OPERAZIONI DI PARCHEGGIO
//////////////////////////////////////////////////////////////////////////////
#define ERROR_PARKING_TILT_SETTING          1
#define ERROR_PARKING_LENZE_BUSY            2
#define ERROR_PARKING_LENZE_TMO             3
#define ERROR_PARKING_LENZE_POSITION        4
#define ERROR_PARKING_ARM_TMO               5
#define ERROR_PARKING_ARM_WRONG_ANGLE       6
#define WARNING_PARKING_ACTIVATION_PROCEDURE       7
#define WARNING_UNPARKING_ACTIVATION_PROCEDURE     8
#define ERROR_PARKING_NOT_CALIBRATED        9


//////////////////////////////////////////////////////////////////////////////
//              CODICI DI ERRORE GENERICO ALR_SOFT
//////////////////////////////////////////////////////////////////////////////
#define ERROR_MCC 1


//////////////////////////////////////////////////////////////////////////////
//              CODICI DI ERRORE MOVIMENTO TUBO
//////////////////////////////////////////////////////////////////////////////
#define TRX_INVALID_ANGOLO      1       // Richiesto movimento troppo ampio
#define TRX_ERROR_ROT_LIM       2       // Movimento Tubo non consentito per rischio impatto


//////////////////////////////////////////////////////////////////////////////
//              CODICI DI ERRORE SEQUENZA RAGGI STANDARD (<200)
//////////////////////////////////////////////////////////////////////////////

#define RXOK                    0
#define ERROR_INVALID_DATA		1 		// VALIDAZIONE DATI DI ESPOSIZIONE
#define ERROR_TMO_XRAY_ENA		2 		// TMO ATTESA XRAY_ENA ON
#define ERROR_TMO_AR			3 		// TMO ATTESA AR OK
#define ERROR_PUSHRX_NO_PREP	4 		// RILASCIO PULSANTE ANTICIPATO NO RAGGI
#define ERROR_AR_BUSY			5		// Anodo Rotante BUSY
#define ERROR_CLOSED_DOOR       6		// La porta dello studio risulta aperta
#define ERROR_WAIT_PENDING_XRAY 7	    // La sequenza raggi precedente non ï¿½ terminata in tempo

#define	LAST_ERROR_NO_PREP		20		// Codici riservati ad errori senza Prep attivo

// ERRORI CON PREP ATTIVO
#define ERROR_DT_EXPWIN			21 		// TMO ATTESA DETECTOR EXP WIN
#define ERROR_PUSHRX_XRAY		22	 	// RILASCIO PULSANTE ANTICIPATO DURANTE RAGGI
#define ERROR_HV_HIGH			23		//Livello HV troppo alta
#define ERROR_HV_LOW			24 		// Livello HV troppo bassa
#define ERROR_IA_HIGH			25              // Livello IA troppo alta
#define ERROR_IA_LOW			26              // Livello IA troppo bassa
#define ERROR_IFIL			    27		// Anomalia corrente di filamento
#define ERROR_VFIL			    28		// Anomalia tensione di filamento
#define ERROR_PWR_HV			29		// Anomalia Alimentazione Alta Tensione
#define ERROR_TMO_RX			30		// Timeout Raggi
#define ERROR_PUSHRX_AFTER_PREP         31              // Rilascio anticipato dopo PREP ATTIVATO
#define ERROR_TRX_FAULT_AFTER_PREP      32              // Anomalia movimento del tubo durante raggi

#define ESPOSIMETRO_INVALID_AEC_DATA  57      // Dati AEC non validi
#define ESPOSIMETRO_BREAST_DENSE  58      // Seno troppo denso
#define ESPOSIMETRO_AEC_SOVRAESPOSTO  59      // Dati expose AEC non disponibili
#define	LAST_ERROR_WITH_PREP    60		// Codici riservati ad errori con Prep attivo

// ---------------- ALTRI CODICI DI ERRORE
#define ERROR_INVALID_KV            61
#define ERROR_INVALID_MAS           62
#define ERROR_INVALID_GEN_CONFIG    63
#define ERROR_NOT_CALIBRATED_KV     64
#define ERROR_NOT_CALIBRATED_I      65
#define ERROR_TUBE_NOT_CONFIGURED   66
#define ERROR_MCC_COMMAND           67
#define ERROR_INVALID_FUOCO         70
#define ERROR_INVALID_FILTRO        71
#define ERROR_INVALID_COLLI         72
#define ERROR_INVALID_CALIB_DATA    73

#define ERROR_VCC                   80		// Anomalia Tensione alimentazione Board
#define ERROR_GRID                  81		// Anomalia tensione Griglia

#define ERROR_ANODE_HU              89      // Anodo Troppo caldo
#define ERROR_SENS_CUFFIA           90      // Anodo Troppo caldo
#define ERROR_CUFFIA_CALDA          91      // La Temperatura della cuffia ï¿½ troppo alta per fare raggi
#define ERROR_MISS_PIOMPO           92      // La sequenza richiede l'accessorio PIOMBO
#define ERROR_MISS_PLEXY            93      // La sequenza richiede l'accessorio PLEXYGLASS
#define ERROR_MISSA_PROT_PAZIENTE   94      // La sequenza richiede l'accessorio Protezione paziente
#define ERROR_MIRROR_LAMP           95      // Errore disattivazione Specchio/Lampada
#define ERROR_MISS_PROT_PAZIENTE_3D 96      // Hotfix 11C

// Errori starter Interno
#define INIT_ALR_STARTER      97
#define ARFLT_MAIN_OFF        INIT_ALR_STARTER    // Errore in fase di OFF
#define ARFLT_SHIFT_OFF       98    // Errore in fase di OFF
#define ARFLT_MAIN_RUN_MAX    99    // Fault durante lancio > MAX
#define ARFLT_MAIN_RUN_MIN    100   // Fault durante lancio < MIN
#define ARFLT_MAIN_KEEP_MAX   101   // Fault durante mantenimento > MAX
#define ARFLT_MAIN_KEEP_MIN   102   // Fault durante mantenimento < MIN
#define ARFLT_SHIFT_RUN_MAX   103   // Fault durante lancio > MAX
#define ARFLT_SHIFT_RUN_MIN   104   // Fault durante lancio < MIN
#define ARFLT_SHIFT_KEEP_MAX  105   // Fault durante mantenimento > MAX
#define ARFLT_SHIFT_KEEP_MIN  106   // Fault durante mantenimento < MIN
#define END_ALR_STARTER       106

#define ERROR_INVALID_CASSETTE      191     // Errore presenza cassetta

#define ERROR_INVALID_SMALL_FOCUS   192     // Utilizzo del fuoco piccolo in esposizioni senza ingranditore
#define ERROR_INVALID_MAG_FUOCO     193     // Ingranditore: si sta usando il fuoco grande per un ingrandimento
#define ERROR_INVALID_MAG_FACTOR    194     // Ingranditore: codice ingrandimento non valido o non configurato
#define ERROR_MISSING_PAD           195     // PAD non riconosciuto o assente
#define ERROR_INVALID_PAD           196     // PAD non idoneo al contesto corrente
#define ERROR_INVALID_POTTER        197     // Potter non riconosciuto e non consentito durante raggi
#define ERROR_ANGOLO_ARM            198     // Errore impostazione angolo non corrispondente
#define ERROR_MISSING_COMPRESSION   199     // Errore mancanza compressione


//////////////////////////////////////////////////////////////////////////////
//              CODICI DI ERRORE COMUNI PER LE SEQUENZE
//////////////////////////////////////////////////////////////////////////////
#define _SEQ_IO_TIMEOUT         200             // Attesa Aggiornamento IO
#define _SEQ_DRIVER_FREEZE      201             // I drive non rispondono alla richiesta di freeze
#define _SEQ_DRIVER_READY       202             // I drive non rispondono alla richiesta di ready
#define _SEQ_UPLOAD190_PARAM    203             // Operazione di Upload su scheda 190 fallita
#define _SEQ_PCB190_BUSY        204             // Attivazione fallita sequenza raggi su PCB190: busy
#define _SEQ_PCB190_TMO         205             // Sequenza raggi in Timeout
#define _SEQ_WAIT_AEC_DATA      206             // Timeout attesa dati AEC
#define _SEQ_AEC_NOT_AVAILABLE  207             // Dati AEC non arrivati
#define _SEQ_READ_REGISTER      208             // Errore su lettura registro
#define _SEQ_WRITE_REGISTER     209             // Errore su scrittura registro
#define _SEQ_ERR_COLLI_TOMO     210             // Errore su comando di modalitï¿½ tomo
#define _SEQ_ERR_WIDE_HOME      211             // Errore su posizionamento in Wide Home
#define _SEQ_ERR_NARROW_HOME    212             // Errore su posizionamento in Narrow Home
#define _SEQ_ERR_TRX_CC         213             // Errore su posizionamento tubo in CC
#define _SEQ_ERR_WIDE_END       214             // Errore su posizionamento in Wide End
#define _SEQ_ERR_NARROW_END     215             // Errore su posizionamento in Narrow End
#define _SEQ_ERR_INTERMEDIATE_HOME    216       // Errore su posizionamento in Intermediate Home
#define _SEQ_ERR_INTERMEDIATE_END     217       // Errore su posizionamento in Intermediate End
#define _SEQ_ERR_INVALID_TUBE_ACTIVATION     218       // Attivazione Tubo non consentita a causa dell'angolo critico

#define _SEQ_UPLOAD_PCB244_A_PARAM  219         // Errore caricamento dati su esposimetro
#define _SEQ_PCB244_A_TMO           220         // Timeout Esposimetro
#define _SEQ_PCB244_A_BUSY          221         // Esposimetro occupato


//___________________________________________________________________________
// CODICI DI ERRORE MOTORI NANOTEC
#define NANOTEC_ERRCLASS_0x1    "General error"
#define NANOTEC_ERRCLASS_0x2    "Current measured error"
#define NANOTEC_ERRCLASS_0x4    "Voltage measured error"
#define NANOTEC_ERRCLASS_0x8    "Temperature measured error"
#define NANOTEC_ERRCLASS_0x10   "Communication error"
#define NANOTEC_ERRCLASS_0x20   "Operating mode error"
#define NANOTEC_ERRCLASS_0x40   "Reserved Error"
#define NANOTEC_ERRCLASS_0x80   "Wrong rotation direction detection"



#define NANOTEC_ERRNUM_0 "Watchdog-Reset"
#define NANOTEC_ERRNUM_1 "Input voltage too high"
#define NANOTEC_ERRNUM_2 "Output current too high"
#define NANOTEC_ERRNUM_3 "Input voltage too low"
#define NANOTEC_ERRNUM_4 "Error at fieldbus"
#define NANOTEC_ERRNUM_5 "Motor turns  in spite of active block – in the wrong direction"
#define NANOTEC_ERRNUM_6 "CANopen only: NMT master takes too long to send nodeguarding request"
#define NANOTEC_ERRNUM_7 "Encoder error due to electrical fault or defective hardware"
#define NANOTEC_ERRNUM_8 "Encoder error; index not found during the auto setup"
#define NANOTEC_ERRNUM_9 "Error in the AB track"
#define NANOTEC_ERRNUM_10 "Positive limit switch and tolerance zone exceeded"
#define NANOTEC_ERRNUM_11 "Negative limit switch and tolerance zone exceeded"
#define NANOTEC_ERRNUM_12 "Device temperature above 80°C"
#define NANOTEC_ERRNUM_13 "The values of object 6065h (Following Error Window) and object 6066h (Following Error Time Out) were exceeded; a fault was triggered."
#define NANOTEC_ERRNUM_14 "Nonvolatile memory full; controller must be restarted for cleanup work."
#define NANOTEC_ERRNUM_15 "Motor blocked"
#define NANOTEC_ERRNUM_16 "Nonvolatile memory damaged; controller must be restarted for cleanup  work."
#define NANOTEC_ERRNUM_17 "CANopen only: Slave took too long to send PDO messages."
#define NANOTEC_ERRNUM_18 "Hall sensor faulty"
#define NANOTEC_ERRNUM_19 "CANopen only: PDO not processed due to a length error"
#define NANOTEC_ERRNUM_20 "CANopen only: PDO length exceeded"
#define NANOTEC_ERRNUM_21 "Nonvolatile memory full; controller must be restarted for cleanup work."
#define NANOTEC_ERRNUM_22 "Rated current must be set (203Bh:01h)"
#define NANOTEC_ERRNUM_23 "Encoder resolution, number of pole pairs and some other values are incorrect."
#define NANOTEC_ERRNUM_24 "Motor current is too high, adjust the PI parameters."
#define NANOTEC_ERRNUM_25 "Internal software error, generic"
#define NANOTEC_ERRNUM_26 "Current too high at digital output"
#define NANOTEC_ERRNUM_27 "CANopen only: Unexpected sync length"
#define NANOTEC_ERRNUM_28 "EtherCAT only: The motor was stopped because EtherCAT switched"
#define NANOTEC_ERRNUM_251 "NA"
#define NANOTEC_ERRNUM_252 "NA"
#define NANOTEC_ERRNUM_253 "Can Bus monitoring"
#define NANOTEC_ERRNUM_254 "Main CPU activity"
#define NANOTEC_ERRNUM_255 "External Safety Device"

#define NANOTEC_ERRCODE_0x0001 "Solenoid safety device detected active"
#define NANOTEC_ERRCODE_0x0002 "Obstacle detection switch active"
#define NANOTEC_ERRCODE_0x0003 "Engine protection cover switch activated"
#define NANOTEC_ERRCODE_0x0004 "Driver disabled by software"
#define NANOTEC_ERRCODE_0x0005 "Unable to read SDO registers"
#define NANOTEC_ERRCODE_0x0006 "Obstacle detection switch blocked"
#define NANOTEC_ERRCODE_0x0007 "NA"
#define NANOTEC_ERRCODE_0x0008 "NA"

#define NANOTEC_ERRCODE_0x1000 "General error"
#define NANOTEC_ERRCODE_0x2300 "Current at the controller output too large"
#define NANOTEC_ERRCODE_0x3100 "Overvoltage/undervoltage at controller input"
#define NANOTEC_ERRCODE_0x4200 "Temperature error within the controller"
#define NANOTEC_ERRCODE_0x6010 "Software reset (watchdog)"
#define NANOTEC_ERRCODE_0x6100 "Internal software error, generic"
#define NANOTEC_ERRCODE_0x6320 "Rated current must be set (203Bh:01h)"
#define NANOTEC_ERRCODE_0x7121 "Motor blocked"
#define NANOTEC_ERRCODE_0x7305 "Incremental encoder or Hall sensor faulty"
#define NANOTEC_ERRCODE_0x7600 "Nonvolatile memory full or corrupt; restart the controller for cleanup work"
#define NANOTEC_ERRCODE_0x8000 "Error during fieldbus monitoring"
#define NANOTEC_ERRCODE_0x8130 "CANopen only: Life Guard error or Heartbeat error"
#define NANOTEC_ERRCODE_0x8200 "CANopen only: Slave took too long to send PDO messages."
#define NANOTEC_ERRCODE_0x8210 "CANopen only: PDO was not processed due to a length error"
#define NANOTEC_ERRCODE_0x8220 "CANopen only: PDO length exceeded"
#define NANOTEC_ERRCODE_0x8611 "Position monitoring error: Following error too large"
#define NANOTEC_ERRCODE_0x8612 "Position monitoring error: Limit switch and tolerance zone exceeded"
#define NANOTEC_ERRCODE_0x9000 "EtherCAT: Motor running while EtherCAT changes from OP -> SafeOp, PreOP, etc."

#define TRX_ERROR_SAFETY_FAULT_CODE         0xFF010001  // Errore relativo al fault
#define TRX_ERROR_SAFETY_OBSTACLE_CODE      0xFF010002  // Errore relativo allo switch di ostacolo
#define ARM_ERROR_SWITCH_SAFETY_CODE        0xFF010003  // Errore relativo allo switch copertura motore

#define TRX_ERROR_BLOCKED_OBSTACLE_CODE     0xFF010006  // Errore relativo allo switch di ostacolo bloccato
#define ARM_ERROR_BLOCKED_OBSTACLE_CODE     0xFF010006  // Errore relativo allo switch di ostacolo bloccato

#define ARM_ERROR_DISABLE_CODE              0xFE010004  // Errore relativo alla disabilitazione del driver da software
#define TRX_ERROR_DISABLE_CODE              0xFE010004  // Errore relativo alla disabilitazione del driver da software
#define TRX_ERROR_COMMUNICATION_CODE        0xFD100005  // Errore relativo alla disabilitazione del driver da software
#define ARM_ERROR_COMMUNICATION_CODE        0xFD100005  // Errore relativo alla disabilitazione del driver da software
#endif
