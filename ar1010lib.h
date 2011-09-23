/* -------------------------------------------------------------------------------
 ar1010lib.h
   
 Version: 0.9
 Created by Adam Jansch on 20/03/2010.
 Library for I2C (TWI) communications from Arduino to AR1010 radio receiver module
 
 Tested with Arduino Duemilanove running an ATmega328
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 -------------------------------------------------------------------------------- */

// Ensure this library description is only included once
#ifndef ar1010lib_h
#define ar1010lib_h

// Include libraries ============================================
#include "WProgram.h"		// Arduino standard types and constants

class AR1010
	{
	public:
		// Constructor/destructor --------------------------------------------------
		AR1010();
		~AR1010();
		
		// Write/read methods -------------------------------------------
		void writeToRegister(uint8_t address, uint16_t data);
		uint16_t readFromRegister(uint8_t address);
		
		// Other methods ------------------------------------------------
		void initialise(uint16_t *customRegisters = NULL);
		
		void setSeekThreshold(uint8_t threshold);
	
		void setFrequency(uint16_t frequency);
		void setVolume(uint8_t volume);
		void setHardmute(bool hmute);
		void setSoftmute(bool smute);
		
		uint16_t frequency();
		uint16_t seek(char direction = 'u');	// 'u' - up, 'd' - down
		
	private:		
		void setBitInRegister(uint8_t address, uint8_t bit, bool bitState);
		void setSeekDirection(char direction);	// 'u' - up, 'd' - down
		
		uint8_t readLOInjection(bool loHi = 0);		// Returns stronger signal - 0 = low-side, 1 = high-side
		void tuneWithAutoHiLo();
	};

#endif