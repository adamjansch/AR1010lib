/* 	-------------------------------------------------------------------------------
	ar1010lib.h
	   
	Version: 0.9
	Created by Adam Jansch on 20/03/2010.
	Library for I2C (TWI) communications from Arduino to AR1010 radio receiver module
	 
	Tested with Arduino Duemilanove running an ATmega328
 
 	Copyright (c) 2010, Adam Jansch
	All rights reserved.
	
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	    * Redistributions of source code must retain the above copyright
	      notice, this list of conditions and the following disclaimer.
	    * Redistributions in binary form must reproduce the above copyright
	      notice, this list of conditions and the following disclaimer in the
	      documentation and/or other materials provided with the distribution.
	    * Neither the name of the names of its contributors may be used to endorse
	      or promote products derived from this software without specific prior
	      written permission.
	
	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL ADAM JANSCH BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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