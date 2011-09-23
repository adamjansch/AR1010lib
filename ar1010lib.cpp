/* ------------------------------------------------------------------------------
 ar1010lib.cpp
 
 Version: 0.9
 Created by Adam Jansch on 20/03/2010.
 Library for I2C (TWI) communications from Arduino to AR1010 radio receiver module
 
 Tested with Arduino Duemilanove running an ATmega328
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 -------------------------------------------------------------------------------- */

#include <Wire.h>			// Library for I2C comms
#include "WProgram.h"		// Arduino standard types and constants
#include "ar1010lib.h"


#define AR1010_ADDRESS (0x4 << 2 | 0x0)  // !! DON'T ADD R/W BIT - WIRE LIBRARY DOES THIS FOR YOU!

#define RSSI_ADDR		0x12	 // Address of the RSSI register
#define ADDR_STATUS		0x13     // Address of the status register
#define CHAN_MASK		0xFE00	 // Bit range of register 2's channel setting
#define SHIFT_READCHAN	7		 // Number of bits to shift to get READCHAN value
 
#define MASK_ST			0x0008   // Stereo D3 in address 13H
#define MASK_READCHAN	0xFF80   // D7~D15 in address 13H

#define HILO_MASK		0x7FFA	 // D15, D2 and D0 in register 11 - hi/lo injection bits

// DEFINED ARRAYS ---------------------------------------------------------
// Array of 2-byte ints to set the radio's registers on start-up
// TAKEN FROM PROGRAMMING GUIDE REV F 0.81
const uint16_t register_init[18] = {
	0xFFFB,     // R0:  1111 1111 1111 1011 
	0x5B15,     // R1:  0101 1011 0001 0101 - Mono (D3), Softmute (D2), Hardmute (D1)  !! SOFT-MUTED BY DEFAULT !!
	0xD0B9,     // R2:  1101 0000 1011 1001 - Tune/Channel
	0xA010,     // R3:  1010 0000 0001 0000 - Seekup (D15), Seek bit (D14), Space 100kHz (D13), Seek threshold: 16 (D6-D0)
	0x0780,     // R4:  0000 0111 1000 0000
	0x28AB,     // R5:  0010 1000 1010 1011
	0x6400,     // R6:  0110 0100 0000 0000
	0x1EE7,		// R7:  0001 1110 1110 0111
	0x7141,		// R8:  0111 0001 0100 0001 
	0x007D,		// R9:  0000 0000 0111 1101
	0x82C6,		// R10: 1000 0010 1100 0110 - Seek wrap (D3)
	0x4E55,		// R11: 0100 1110 0101 0101
	0x970C,		// R12: 1001 0111 0000 1100
	0xB845,		// R13: 1011 1000 0100 0101
	0xFC2D,		// R14: 1111 1100 0010 1101 - Volume control 2 (D12-D15)
	0x8097,		// R15: 1000 0000 1001 0111
	0x04A1,		// R16: 0000 0100 1010 0001
	0xDF61		// R17: 1101 1111 0110 0001
};

// Recommended volume settings
const uint8_t volume_map[19] = {
	0x0F, 0xCF, 0xDF, 0xFF, 0xCB,
	0xDB, 0xFB, 0xFA, 0xF9, 0xF8,
	0xF7, 0xD6, 0xE6, 0xF6, 0xE3,
	0xF3, 0xF2, 0xF1, 0xF0
};

// Define register/bit arrays for particular functions
const uint8_t hardmute_bit[2] = { 1, 1 };		// Register 1 -  xxxx xxxx xxxx xxDx
const uint8_t softmute_bit[2] = { 1, 2 };		// Register 1 -  xxxx xxxx xxxx xDxx
const uint8_t seek_bit[2] = { 3, 14 };			// Register 3 -  xDxx xxxx xxxx xxxx
const uint8_t seekup_bit[2] = { 3, 15 };		// Register 3 -  Dxxx xxxx xxxx xxxx
const uint8_t tune_bit[2] = { 2, 9 };			// Register 2 -  xxxx xxDx xxxx xxxx
const uint8_t hiloctrl1_bit[2] = { 11, 2 };		// Register 11 - xxxx xxxx xxxx xDxx
const uint8_t hiloctrl2_bit[2] = { 11, 0 };		// Register 11 - xxxx xxxx xxxx xxxD
const uint8_t hiloside_bit[2] = { 11, 15 };		// Register 11 - Dxxx xxxx xxxx xxxx



// Constructor ============================================================
AR1010::AR1010()
{
	// Nada
}

// Destructor =============================================================
AR1010::~AR1010()
{
	// Nada
}


// Write/read methods -----------------------------------------------------
// Write method ===========================================================
void AR1010::writeToRegister(uint8_t address, uint16_t data)
{
	// Write [data] to register at [address] ------------------------------
	Wire.beginTransmission(AR1010_ADDRESS);  // START condition
	Wire.send(address);
	
	Wire.send(uint8_t((data & 0xFF00) >> 8));
	Wire.send(uint8_t(data & 0x00FF));
	
	Wire.endTransmission();
}

// Read method ============================================================
uint16_t AR1010::readFromRegister(uint8_t address)
{
	uint8_t receive_msb = 0, receive_lsb = 0;
	uint16_t received_data;
	
	// Establish connection, select receipt address
	Wire.beginTransmission(AR1010_ADDRESS);
	Wire.send(address);
	Wire.endTransmission();
	
	// Request two data bytes
	Wire.requestFrom(AR1010_ADDRESS, 2);
	
	// Fill variables when ready
	while(Wire.available())
	{
		receive_msb = Wire.receive();
		receive_lsb = Wire.receive();
	}
	
	// Reconstitute 2-byte register into uint16_t
	received_data = (receive_msb << 8) + receive_lsb;
	
	return received_data;
}


// Other methods ----------------------------------------------------------
// Initialise AR1010 - fill all writeable registers =======================
void AR1010::initialise(uint16_t *customRegisters)
{
	// customRegisters set as NULL by default
	
	// Write registers 1-17 -----------------------------------------------
	for(uint8_t i = 1; i < 18; i++)
	{
		if(customRegisters == NULL)
			writeToRegister(i, register_init[i]);
		else
			writeToRegister(i, customRegisters[i]);
	}
		
	
	// Following 1-17 write register 0,   ---------------------------------  
	if(customRegisters == NULL)
		writeToRegister(0, register_init[0]);
	else
		writeToRegister(0, customRegisters[0]);
}


// Set seek threshold =================================================================
void AR1010::setSeekThreshold(uint8_t threshold)
{
	uint16_t temp = readFromRegister(3) & 0xFF80;	// Get register data and clear affected bits (D6-D0)
	writeToRegister(3, temp | threshold);			// Set threshold value and write back to register
}


// Set radio frequency ================================================================
void AR1010::setFrequency(uint16_t frequency)
{
	uint16_t register2, channel = frequency - 690;
	
	// Correct out of lower range
	if(channel < 185)
		channel = 185;
	else if(channel > 390)
		channel = 390;
	
	setBitInRegister(tune_bit[0], tune_bit[1], 0);	// Set tune bit
	
	// Set radio channel
	register2 = readFromRegister(2) & CHAN_MASK;
	register2 |= channel;
	writeToRegister(2, register2);
	
	setBitInRegister(tune_bit[0], tune_bit[1], 1);	// Set tune bit
}

// Set volume using the defined volume values =========================================
void AR1010::setVolume(uint8_t volume)
{
	// Put volume value in range 0 - 18
	uint8_t temp_vol = volume;
	
	if(temp_vol < 0)
		temp_vol = 0;
	else if(temp_vol > 18)
		temp_vol = 18;
	
	// Volume values are held in registers 3 (D7-10) and 14 (D12-15)
	uint16_t register3, register14;
	uint8_t volume_setting = volume_map[temp_vol];
	
	register3 = readFromRegister(3) & 0xF87F;		// Zero the bits to change (D7-10)
	register3 |= ((volume_setting & 0x0F) << 7);    // Place 4 LSBs of volume at D7-10
	
	register14 = readFromRegister(14) & 0x0FFF;		// Zero the bits to change (D12-15)
	register14 |= ((volume_setting & 0xF0) << 8);   // Place 4 MSBs of volume at D12-15
	
	writeToRegister(3, register3);
	writeToRegister(14, register14);
}

// Set the hardmute function ==========================================================
void AR1010::setHardmute(bool hmute)
{ 
	setBitInRegister(hardmute_bit[0], hardmute_bit[1], hmute);
}

// Set the softmute function ==========================================================
void AR1010::setSoftmute(bool smute)
{ 
	setBitInRegister(softmute_bit[0], softmute_bit[1], smute);
}


// Get current channel ================================================================
uint16_t AR1010::frequency()
{
	return(((readFromRegister(ADDR_STATUS) & MASK_READCHAN) >> SHIFT_READCHAN) + 690);
}


// Seek method ============================================================
uint16_t AR1010::seek(char direction)
{
	/*
	 PSEUDO-CODE FROM PROGRAMMING GUIDE
	 1 Set hmute Bit 
	 2 Clear TUNE Bit 
	 3 Set CHAN Bits 
	 4 Clear SEEK Bit 
	 5 Set SEEKUP/SPACE/BAND/SEEKTH Bits 
	 6 Enable SEEK Bit 
	 7 Wait STC flag (Seek/Tune Complete, in “Status” register) 
	 8 If SF then tune with AutoHiLo
	 9 Clear hmute Bit 
	 10 Update  Functions  (optional,  but  remember  to  update  CHAN  with  the  seek in READCHAN before next seek) 
	 */
	
	uint16_t temp = 0, curChannel;
	
	// 1 May put in later
	
	// 2 Unset tune bit ------------------------------------------------------
	setBitInRegister(tune_bit[0], tune_bit[1], 0);
	
	// 3 is not required - no override required
	
	// 4 Unset seek bit ------------------------------------------------------
	setBitInRegister(seek_bit[0], seek_bit[1], 0);	
	
	// 5 Seek attributes - can be set by other functions ---------------------
	setSeekDirection(direction);
	
	// 6 Set seek bit to enable seek -----------------------------------------
	setBitInRegister(seek_bit[0], seek_bit[1], 1);
	
	// 7 Wait for tune to stabilize (STC flag)
	while(temp == 0)
		temp = readFromRegister(ADDR_STATUS) & 0x0020;
	
	// 8 Check if tune was successful, tune with Auto Hi Lo if not
	temp = readFromRegister(ADDR_STATUS) & 0x0010;
	
	if(temp != 0)
		tuneWithAutoHiLo();	

	// 9 May put in later
	
	// 10 Update CHAN from result (READCHAN) ------------------------------------
	curChannel = frequency() - 690;
	temp = readFromRegister(2) & CHAN_MASK;
	temp |= curChannel;
	writeToRegister(2, temp);
	
	return frequency();
}


// PRIVATE METHODS -------------------------------------------------------
// Set one bit of a given register =======================================
void AR1010::setBitInRegister(uint8_t address, uint8_t bit, bool bitState)
{
	uint16_t temp;
	
	// Use bitState to decide which masking to use (to 1 or to 0)
	if(bitState)
		temp = readFromRegister(address) | (1 << bit);
	else
		temp = readFromRegister(address) & ~(1 << bit);
	
	writeToRegister(address, temp);
}


// Set seek direction =================================================================
void AR1010::setSeekDirection(char direction)
{
	// Change the direction of the seek function: 'u' - up, 'd' - down
	if(direction == 'd')
		setBitInRegister(seekup_bit[0], seekup_bit[1], 0);
	else if(direction == 'u')
		setBitInRegister(seekup_bit[0], seekup_bit[1], 1);
}


// Read low-side or high-side LO injection data =======================================
uint8_t AR1010::readLOInjection(bool loHi)
{
	/*  PSEUDO-CODE FOR READING LO/HI INJECTION
		1. Set R11 (clear D15, clear D0/D2) 
		2. Enable TUNE Bit 
		3. Wait STC flag (Seek/Tune Complete, in “Status” register) 
		4. Get RSSI 
		5. Clear TUNE Bit
		6. Return RSSI
	 */
	
	uint16_t temp = 0;
	uint8_t rssi_val = 0;
	
	// 1. Set R11 - D15, D2-D0 - clear for low-side, set for high-side
	if(loHi)
		temp = readFromRegister(11) | ~(HILO_MASK);
	else
		temp = readFromRegister(11) & HILO_MASK;
	
	writeToRegister(11, temp);
	
	// 2. Set tune bit -----------------------------
	setBitInRegister(tune_bit[0], tune_bit[1], 1);
	
	// 3. Wait for STC flag to stabilise -----------
	while(temp == 0)
		temp = readFromRegister(ADDR_STATUS) & 0x0020;
	
	// 4. Get RSSI value (R0x12, D9-D15)
	rssi_val = (readFromRegister(RSSI_ADDR) & 0xFE00) >> 9;
	
	// 5. Clear tune bit -----------------------------
	setBitInRegister(tune_bit[0], tune_bit[1], 0);
	
	return rssi_val;
}

// Tune with auto high-side/low-side injection ========================================
void AR1010::tuneWithAutoHiLo()
{
	// * A WORK IN PROGRESS!! *
	
	/* PSEUDO-CODE from programming guide
	(1) Set hmute Bit
	(2) Clear TUNE Bit
	(3) Clear SEEK Bit 
	(4) Set BAND/SPACE/CHAN Bits 
	(5) Read Low-side LO Injection
		1. Set R11 ( clear D15, clear D0/D2 ) 
		2. Enable TUNE Bit 
		3. Wait STC flag (Seek/Tune Complete, in “Status” register) 
		4. Get RSSI (RSSI1) 
		5. Clear TUNE Bit 
	(6) Read High-side LO Injection 
		1. Set R11( set D15, set D0/D2 )
		2. Enable TUNE Bit 
		3. Wait STC flag (Seek/Tune Complete, in “Status” register) 
		4. Get RSSI (RSSI2) 
		5. Clear TUNE Bit 
	(7) Compare Hi/Lo Side Signal Strength 
		1. If (RSSI2 > RSSI1) Set R11( clear D15, set D0/D2), else Set R11( set D15, clear D0/D2 ) 
	(8)  Enable TUNE Bit
	(9)  Wait STC flag (Seek/Tune Complete, in “Status” register) 
	(10) Clear hmute Bit 
	(11) Update Functions (optional) 
	 */
	
	uint8_t rssi_lo, rssi_hi;
	uint16_t temp = 0, curChannel;
	
	// 1. Set hardware mute
	
	// 2. Clear tune bit ------------------------------------------------------
	setBitInRegister(tune_bit[0], tune_bit[1], 0);
	
	// 3. Clear seek bit ------------------------------------------------------
	setBitInRegister(seek_bit[0], seek_bit[1], 0);
	
	// 4. Set CHAN from READCHAN ----------------------------------------------
	curChannel = frequency() - 690;
	temp = readFromRegister(2) & CHAN_MASK;
	temp |= curChannel;
	writeToRegister(2, temp);
	
	// 5. Read low-side LO injection ------------------------------------------
	rssi_lo = readLOInjection(0);
	
	// 6. Read high-side LO injection -----------------------------------------
	rssi_hi = readLOInjection(1);
	
	// 7. Compare signal strength, set bits accordingly -----------------------
	if(rssi_hi > rssi_lo)
	{
		// Set D0 and D2, clear D15 ----------------------------
		setBitInRegister(hiloctrl1_bit[0], hiloctrl1_bit[1], 1);
		setBitInRegister(hiloctrl2_bit[0], hiloctrl2_bit[1], 1);
		setBitInRegister(hiloside_bit[0], hiloside_bit[1], 0);
	}
	else
	{
		// Clear D0 and D2, set D15 ----------------------------
		setBitInRegister(hiloctrl1_bit[0], hiloctrl1_bit[1], 0);
		setBitInRegister(hiloctrl2_bit[0], hiloctrl2_bit[1], 0);
		setBitInRegister(hiloside_bit[0], hiloside_bit[1], 1);
	}
	
	// 8. Enable tune bit -----------------------------------------------------
	setBitInRegister(tune_bit[0], tune_bit[1], 1);
	
	// 9. Wait for STC flag to stabilise --------------------------------------
	while(temp == 0)
		temp = readFromRegister(ADDR_STATUS) & 0x0020;
	
	// 10. Clear hardware mute
}