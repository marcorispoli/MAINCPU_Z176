// In this NanoJ program, we will make the motor turn shortly back and forth.
// The language used for NanoJ programs is C, with a few specific extensions,
// like the mappings (see below).
// Please refer to the product manual for more details about NanoJ and the
// object dictionary.

// map U16 ControlWord as inout 0x6040:00
// map U32 ExpWin as input 0x60FD:00
// map S32 Posizione as input 0x6064:00
// map U32 InputActive as inout 0x3240:02

// Include the definition of NanoJ functions and symbols
#include "wrapper.h"

// The user() function is the entry point of the NanoJ program. It is called
// by the firmware of the controller when the NanoJ program is started.
void user()
{
	U08 samples = 0;
	S32 sample;
	
	// Impostazione logica invertita per il pin EXP-WIN (DIG1)
	U32 logica = InOut.InputActive;
	InOut.InputActive = logica | 0x1; // Imposta il solo DIG1 a logica negativa
	yield();
	
	// Attende attivazione Input 
	while((In.ExpWin & 0x10000) != 0x10000) yield();

	// Partenza braccio	
	InOut.ControlWord = 0x2F;							// reset start bit 4, new target position must be acknowledged as new set point immediately(Bit 5)
	yield();
	InOut.ControlWord = 0x3F;							// starts the absolute positioning 	
	yield();
	
	// Debounse 100ms
	sleep(100);
	
	// Ciclo di campionamento
	samples = 1 ;
	while(1){
		// Attesa reset EXP-WIN
		while((In.ExpWin & 0x10000) == 0x10000) yield();
		// Debounse 100ms
		sleep(100);
	
		// Attesa set EXP-WIN
		while((In.ExpWin & 0x10000) != 0x10000) yield();
	
		// Campionamento
		sample = In.Posizione;
		od_write(0x2500, samples, sample );
		yield();
		samples++;
		
		if(samples>32){
			// Raggiunto limite campionamento: fine programma
			od_write(0x2300, 0x00, 0 );
			yield();
		}

		// Debounse 100ms
		sleep(100);
	}
	
	
}

