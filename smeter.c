/*
*	Project: 23cm-NBFM-Transceiver
*	Developer: Bas, PE1JPD
*
*	Module: smeter.c
*	Last change: 18.10.20
*
*	Description: S-meter and RSSI
*/


#include <avr/io.h>
#include <stdio.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "23nbfm.h"
#include <util/delay.h>

extern int tx;
extern int squelchlevel;
extern int mode;

extern int calibration;													// wm
extern int para_m;														// wm
extern int para_c;														// wm

// define S-meter chars
unsigned char smeterChars[4][8] = {										// wm, [3][8]
	{0b00000,0b00000,0b10000,0b10000,0b10000,0b10000,0b00000,0b00000},
	{0b00000,0b00000,0b10100,0b10100,0b10100,0b10100,0b00000,0b00000},
	{0b00000,0b00000,0b10101,0b10101,0b10101,0b10101,0b00000,0b00000},
	{0b00000,0b00000,0b00100,0b00100,0b00100,0b01110,0b00100,0b00000}	// wm little target
};


void createSmeterChars()
{
	// define custom chars for s-meter
	int i, j;

	lcdCmd(0x40);
	for (i=0; i<4; i++) {											// wm, i<3
		for (j=0; j<8; j++) {
			lcdData(smeterChars[i][j]);
		}
	}
}


int readRSSI()
{
	register int rssi;
	int rssi_; 

	ADCSRA |= (1<<ADSC)|(1<<ADEN); 

	while ((ADCSRA & (1<<ADSC))!=0);								// wait for result of conversion

	// calculate Smeter
	rssi = (1024-ADC);												// -44, RSSIoff moves RSSI
	// if (rssi<0) rssi=0;											// wm moved
	
	// mute when tx or squelched
	rssi_ = rssi-SQUELCHoff;										// wm
	if (rssi_<0) rssi_ = 0;											// wm
	if (tx || rssi_<squelchlevel || mode==SPECTRUM) {				// wm
		sbi(PORTC, MUTE);
	}
	else {
		cbi(PORTC, MUTE);

//		analog S output on Timer1, not implemented yet!				// wm not used anymore
//		OCR1B = rssi;
	}

	return rssi;
}


void displaySmeter(int rssi, int x, int y) 
{
	short n = 16;													// RSSI-Bar max. 16 chars
	int s = rssi-RSSIoff;											// wm
	
	if (s<0) s=0;													// wm

	lcdCursor(x, y);												// wm

	#ifdef LCD_20x4													// wm
		// goto fourth line on lcd
		lcdStr("S-M:");
	#endif

	// chars in the full bar are 3 vertical lines
	while(s>=3 && n>0) {
		lcdData(2);
		s -= 3;
		n--;
	}
	
	// last char 0, 1 or 2 lines
	switch (s) {
		case 2: 
			lcdData(1);
			break;
		case 1:
			lcdData(0);
			break;
		default:
			lcdData(' ');
			break;
	}

	// clear any chars to the right (when tx, clear all)
	n--;															// wm, Line overflow to 0,0!
	while (n-->0) lcdData(' ');
}


void displayRSSI(int rssi, int x, int y) 							// wm
{
	char str[20];
	
	lcdCursor(x, y);
	
	if (calibration==FALSE)														
	{	
//		int s = rssi-44;											// wm, RSSIoff, "rssi-44" in dBm calibrated for original value
		int s = rssi;
	
//		if (s<0) s=0;												// wm
		if (s<RSSIoff) s=0;											// wm
	
		if (s==0) {
			#ifdef LCD_20x4	
				lcdStr("RSSI:               ");
			#else
				lcdStr("RSSI:           ");
			#endif
		}
		else
		{															// Normal Mode
			// RSSI[dBm] = s * m + c; m = 1,73; c = -98,35 calculated with spreadsheet (Numbers)
			// Lin 1: m = 1,73 --> 173; c = -98,35 --> 9835
			// Lin 2: m = 1,52 --> 152; c = -98,14 --> 9814
			// all values*100 --> Integer
		
			s = para_c - (para_m * s);								// s = 9814 - (152 * s);									
		
			sprintf(str, "RSSI:%4d.%02d dBm", (int)(s/100*-1), (int)(s%100));
			lcdStr(str);
		}
	}
	else
	{																// Calibrate Mode, raw RSSI
		sprintf(str, "RSSI raw: %d", rssi);							// rssi = (1024-ADC)
		lcdStr(str);
	}
}