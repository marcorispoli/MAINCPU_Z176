#ifndef ANALOG__CALIB_H
#define ANALOG_CALIB_H



// ______________________________________________________________________________________
// DATABASE PER TUTTE LE CALIBRAZIONI
// ______________________________________________________________________________________
#define _DB_XRAYPUSH_READY    _DB_SERVICE1_INT // Campo selezionato
#define _DB_STOP_ATTESA_DATI  _DB_SERVICE2_INT

// ______________________________________________________________________________________
// DATABASE PER CALIBRAZIONE DETECTOR
// ______________________________________________________________________________________

#define _DB_DETECTOR_CALIB_CAMPO    _DB_SERVICE5_INT // Campo selezionato
#define _DB_DETECTOR_RAD            _DB_SERVICE6_INT // Radiazione misurata
#define _DB_DETECTOR_OFSET          _DB_SERVICE7_INT // Radiazione ofset detector (Rx25)
#define _DB_DETECTOR_REF_RAD        _DB_SERVICE8_INT // Reference di calibrazione
#define _DB_DETECTOR_PLOG           _DB_SERVICE9_INT // PLOG rilevato
#define _DB_DETECTOR_DMAS           _DB_SERVICE10_INT // dmAs come risultato di esposizione
#define _DB_REF_TOL                 _DB_SERVICE11_INT // Tolleranza ammessa sulla verifica
#define _DB_READY_STAT              _DB_SERVICE12_INT // Bit di stato per il ready (visualizzazione grafica avvisi)

// ______________________________________________________________________________________
// DATABASE PER CALIBRAZIONE PROFILE
// ______________________________________________________________________________________

#define _DB_CALIB_PROFILE_WINDOW          _DB_SERVICE5_INT
#define _DB_CALIB_PROFILE_NAME            _DB_SERVICE6_STR
#define _DB_CALIB_PROFILE_KVPRE           _DB_SERVICE7_INT
#define _DB_CALIB_PROFILE_DKV             _DB_SERVICE8_INT
#define _DB_CALIB_PROFILE_DMAS_PRE        _DB_SERVICE9_INT
#define _DB_CALIB_PROFILE_DMAS            _DB_SERVICE10_INT
#define _DB_CALIB_PROFILE_PULSE           _DB_SERVICE11_INT
#define _DB_CALIB_PROFILE_DOSE            _DB_SERVICE12_INT

#define _DB_CALIB_PROFILE_CAMPO           _DB_SERVICE13_INT
#define _DB_CALIB_PROFILE_FILTRO          _DB_SERVICE14_INT
#define _DB_CALIB_PROFILE_PLOG            _DB_SERVICE15_INT
#define _DB_CALIB_PROFILE_RAD             _DB_SERVICE16_INT
#define _DB_CALIB_PROFILE_OFFSET          _DB_SERVICE17_INT

#define _DB_CALIB_PROFILE_PMMI            _DB_SERVICE18_INT
#define _DB_CALIB_PROFILE_PC_POTTER       _DB_SERVICE19_INT
#define _DB_CALIB_PROFILE_READY_STAT      _DB_SERVICE20_INT

// ______________________________________________________________________________________
// DATABASE PER CALIBRAZIONE TUBO
// ______________________________________________________________________________________

#define _DB_CALIB_TUBE_READY_STAT         _DB_SERVICE5_INT
#define _DB_CALIB_TUBE_NAME               _DB_SERVICE6_STR
#define _DB_CALIB_TUBE_FOCUS              _DB_SERVICE7_INT
#define _DB_CALIB_TUBE_FILTER             _DB_SERVICE8_INT
#define _DB_CALIB_TUBE_KV                 _DB_SERVICE9_INT
#define _DB_CALIB_TUBE_VDAC               _DB_SERVICE10_INT
#define _DB_CALIB_TUBE_IA                 _DB_SERVICE11_INT
#define _DB_CALIB_TUBE_IDAC               _DB_SERVICE12_INT
#define _DB_CALIB_TUBE_DMAS               _DB_SERVICE13_INT

#define _DB_CALIB_TUBE_DKVR               _DB_SERVICE14_INT
#define _DB_CALIB_TUBE_IAR                _DB_SERVICE15_INT
#define _DB_CALIB_TUBE_DMASR              _DB_SERVICE16_INT

// ______________________________________________________________________________________
// DATABASE PER PANNELLO ESPOSIZIONE MANUALE
// ______________________________________________________________________________________


#define _DB_MANUAL_CAMPO                    _DB_SERVICE5_INT
#define  _DB_MANUAL_KV                      _DB_SERVICE6_INT
#define  _DB_MANUAL_MAS                     _DB_SERVICE7_INT
#define  _DB_MANUAL_FOCUS                   _DB_SERVICE8_INT
#define  _DB_MANUAL_FILTER                  _DB_SERVICE9_INT
#define  _DB_MANUAL_RAD                     _DB_SERVICE10_INT
#define  _DB_MANUAL_PLOG                    _DB_SERVICE11_INT
#define  _DB_MANUAL_PULSE                   _DB_SERVICE12_INT
#define  _DB_MANUAL_KVREAD                  _DB_SERVICE13_INT
#define  _DB_MANUAL_MASREAD                 _DB_SERVICE14_INT
#define  _DB_MANUAL_IAREAD                  _DB_SERVICE15_INT
#define  _DB_MANUAL_PULSE_TIME              _DB_SERVICE16_INT
#define  _DB_MANUAL_READY_STAT              _DB_SERVICE17_INT

#define  _DB_MANUAL_VDAC                    _DB_SERVICE18_INT
#define  _DB_MANUAL_IDAC                    _DB_SERVICE19_INT
#define  _DB_MANUAL_IN                      _DB_SERVICE20_INT
#define  _DB_MANUAL_PRIMO_FILTRO            _DB_SERVICE21_INT
#define  _DB_MANUAL_SECONDO_FILTRO          _DB_SERVICE22_INT
#define  _DB_MANUAL_MAX_KV                  _DB_SERVICE23_INT
#define  _DB_MANUAL_MAX_DMAS                _DB_SERVICE24_INT
#define  _DB_INVALID_FRAME                  _DB_SERVICE25_INT
#define  _DB_MANUAL_KERMA                   _DB_SERVICE26_INT

// ______________________________________________________________________________________
// DATABASE PER PANNELLO COLLIMAZIONE
// ______________________________________________________________________________________
#define _DB_COLLI_CALIB_SEL_PAD              _DB_SERVICE5_INT
#define _DB_COLLI_CALIB_FRONT                _DB_SERVICE6_INT
#define _DB_COLLI_CALIB_BACK                 _DB_SERVICE7_INT
#define _DB_COLLI_CALIB_LEFT                 _DB_SERVICE8_INT
#define _DB_COLLI_CALIB_RIGHT                _DB_SERVICE9_INT
#define _DB_COLLI_CALIB_MIRROR               _DB_SERVICE10_INT

#define _DB_COLLI_DOWNLOAD                   _DB_SERVICE11_INT
#define _DB_COLLI_FACTORY_DOWNLOAD           _DB_SERVICE12_INT
#define _DB_COLLI_MIRROR_DOWNLOAD            _DB_SERVICE13_INT


#define _DEF_COLLI_CALIB_FORMAT_REFERENCE       0
#define _DEF_COLLI_CALIB_FORMAT_24x30           1
#define _DEF_COLLI_CALIB_FORMAT_18x24           2
#define _DEF_COLLI_CALIB_FORMAT_BIOP            3
#define _DEF_COLLI_CALIB_FORMAT_MAG             4
#define _DEF_COLLI_CALIB_FORMAT_MIRROR          5
#define _DEF_COLLI_CALIB_FORMAT_FACTORY         6

#define _DEF_COLLI_CALIB_FORMAT_REF_ITEM       _DEF_COLLI_CALIB_FORMAT_18x24

#endif
