/*
*	Project: 23cm-NBFM-Transceiver
*	Developer: Bas, PE1JPD
*
*	Module: settings.c
*	Last change: 08.10.20
*
*	Description: All code to store and load settings to EEPROM.
*/

/*
*	EEPROM Usage
*
*	Address		Type		Variable
*	-----------------------------------------------------------------
*	Global Settings:
*	      0		int			MAGICNUMBER
*		  2		int			squelchlevel
*		  4		int			mode
*		  6		int			lastSelectedMemory
*		  8		int			Step
*		 10		int			frqadj
*		 12		int			para_m
*		 14		int			para_c
*
*	Memory Settings for M0..M9 (address = 100 + memory*10):
*	M0:
*		100		long int	freq
*		104		int			tone
*		106		int			shift
*	M1:
*		110		long int	freq
*		114		int			tone
*		116		int			shift
*	M2:
*	...
*/


#include <avr/io.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "23nbfm.h"
#include <util/delay.h>

#define MAGICNUMBER 18296											// if not found, clear eeprom for new GlobalSettings

extern int squelchlevel;
extern int mode;

extern long int freq;
extern int shift, step;

#ifdef ADF4153														// wm
extern int frqadj;
#endif

extern int tone;
extern int lastSelectedMemory;

extern int para_m;													// wm
extern int para_c;													// wm


// write default values in memories
void clearMemory()
{
	squelchlevel = 0;
	mode = VFO;
	lastSelectedMemory = 0;
	step = 25;
	writeGlobalSettings();

	freq = 1298000UL;
	
	#ifdef TONE_1750												// wm
		tone = 598;													// CTCSS off with 1750 HZ
	#else
		tone = 599;													// CTCSS off without 1750 Hz
	#endif
	
	shift = 0;
	
	#ifdef ADF4153													// wm
		frqadj = 0;
	#endif

	for (int mem=0; mem<=MAXMEM; mem++) {
		writeMemory(mem);
	}
}


// Read global settings
void readGlobalSettings()
{
	unsigned int address = 0;

	int magic = eeprom_read_word((unsigned int *)address);
	if (magic != MAGICNUMBER) {
		clearMemory();
	}
	else {
		address += 2;
		squelchlevel = eeprom_read_word((unsigned int *)address);
		address += 2;
		mode = eeprom_read_word((unsigned int *)address);
		address += 2;
		lastSelectedMemory = eeprom_read_word((unsigned int *)address);
		address += 2;
		step = eeprom_read_word((unsigned int *)address);
		address += 2;
		
		#ifdef ADF4153												// wm, get Frequency Adjust
			frqadj = eeprom_read_word((unsigned int *)address);
			address += 2;
		#endif
		
		para_m = eeprom_read_word((unsigned int *)address);			// wm parameter for linear regression
		address += 2;
		para_c = eeprom_read_word((unsigned int *)address);			// wm parameter for linear regression
		address += 2;
	}
}


void writeGlobalSettings()
{
	unsigned int address = 0;
	int eeprom_word;

	eeprom_word = eeprom_read_word((unsigned int *)address);		// wm, write only to eeprom, if value has changed
	if (eeprom_word!=MAGICNUMBER)
		eeprom_write_word((unsigned int *)address, (int)MAGICNUMBER);
	address += 2;
	
	eeprom_word = eeprom_read_word((unsigned int *)address);
	if (eeprom_word!=squelchlevel)
		eeprom_write_word((unsigned int *)address, (int)squelchlevel);
	address += 2;
	
	eeprom_word = eeprom_read_word((unsigned int *)address);
	if (eeprom_word!=mode)
		eeprom_write_word((unsigned int *)address, (int)mode);
	address += 2;
	
	eeprom_word = eeprom_read_word((unsigned int *)address);
	if (eeprom_word!=lastSelectedMemory)
		eeprom_write_word((unsigned int *)address, (int)lastSelectedMemory);
	address += 2;
	
	eeprom_word = eeprom_read_word((unsigned int *)address);
	if (eeprom_word!=step)
		eeprom_write_word((unsigned int *)address, (int)step);
	address += 2;
	
	#ifdef ADF4153													// wm, store Frequency Adjust
		eeprom_word = eeprom_read_word((unsigned int *)address);
		if (eeprom_word!=frqadj)
			eeprom_write_word((unsigned int *)address, (int)frqadj);
		address += 2;	
	#endif

	eeprom_word = eeprom_read_word((unsigned int *)address);		// wm
	if (eeprom_word!=para_m)
	eeprom_write_word((unsigned int *)address, (int)para_m);
	address += 2;
	
	eeprom_word = eeprom_read_word((unsigned int *)address);		// wm
	if (eeprom_word!=para_c)
	eeprom_write_word((unsigned int *)address, (int)para_c);
	address += 2;	
}


// Read memory settings from EEPROM
void readMemory(int memory)
{
	unsigned int address = 0;

	if (memory<0 || memory>MAXMEM) {
		return;
	}
  
	address = 100 + memory*10;

	freq = eeprom_read_dword((unsigned long int *)address);
	address += 4;
	tone = eeprom_read_word((unsigned int *)address);
	address += 2;
	shift = eeprom_read_word((unsigned int *)address);
	address += 2;
}


// Write memory settings to EEPROM
void writeMemory(int memory)
{
	if (memory<0 || memory>MAXMEM) {
  		return;
	}

	unsigned int address = 100 + memory*10;
	int eeprom_word;												// wm
	long int eeprom_dword;											// wm

	eeprom_dword = eeprom_read_dword((unsigned long int *)address);	// wm, write only to eeprom, if value has changed
	if (eeprom_dword!=freq)
		eeprom_write_dword((unsigned long int *)address, (long int)freq);
	address += 4;

	eeprom_word = eeprom_read_word((unsigned int *)address);
	if (eeprom_word!=tone)
		eeprom_write_word((unsigned int *)address, (int)tone);
	address += 2;
	
	eeprom_word = eeprom_read_word((unsigned int *)address);
	if (eeprom_word!=shift)
		eeprom_write_word((unsigned int *)address, (int)shift);		// wm
	address += 2;
}


