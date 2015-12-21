/*
Copyright (c) 2015, Los Alamos National Security, LLC
All rights reserved.

Copyright 2015. Los Alamos National Security, LLC. This software was
produced under U.S. Government contract DE-AC52-06NA25396 for Los
Alamos National Laboratory (LANL), which is operated by Los Alamos
National Security, LLC for the U.S. Department of Energy. The
U.S. Government has rights to use, reproduce, and distribute this
software.  NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY,
LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY
FOR THE USE OF THIS SOFTWARE.  If software is modified to produce
derivative works, such modified software should be clearly marked, so
as not to confuse it with the version available from LANL.

Additionally, redistribution and use in source and binary forms, with
or without modification, are permitted provided that the following
conditions are met:

• Redistributions of source code must retain the above copyright
         notice, this list of conditions and the following disclaimer.

• Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer
         in the documentation and/or other materials provided with the
         distribution.

• Neither the name of Los Alamos National Security, LLC, Los Alamos
         National Laboratory, LANL, the U.S. Government, nor the names
         of its contributors may be used to endorse or promote
         products derived from this software without specific prior
         written permission.

THIS SOFTWARE IS PROVIDED BY LOS ALAMOS NATIONAL SECURITY, LLC AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LOS
ALAMOS NATIONAL SECURITY, LLC OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.*/

//*****************************************************************************
//
// AUTHOR:  Heather Quinn
// CONTACT INFO:  hquinn at lanl dot gov
// LAST EDITED: 12/21/15
//
// cache_test.c
//
// This test is a simple program for instrumenting SRAM memory blocks or caches.
// It tests the foura mostly 0s memory pattern.  A simple sum of
// memory array elements is also done to check for transients in logic.
// Keep in mind that the sum checker is hardcoded.  If the memory arrays change
// size or values, it is necessary to change the sum values, too.
//
// This software is otimized for TI microcontrollers, specifically the
// MSP430F2619.
//
// The output is designed to go out the UART at a speed of 9,600 baud and uses a
// tiny printf to reduce printf footprint.  The tiny printf can be downloaded from 
// http://www.43oh.com/forum/viewtopic.php?f=10&t=1732  All of the output is YAML
// parsable.
//
//
//*****************************************************************************


#include <msp430.h>
#include <string.h>

void sendByte(char);
void printf(char *, ...);
void initUART(void);
void initMSP430();
void cache_test(void);
void init_array(int array[]);
int calc_sum(int array[]);

#define 	robust_printing			1
#define		data_array_elements		600
#define		sum_const				-16908

int array[data_array_elements];

unsigned long int ind = 0;
int local_errors = 0;
int sum_errors = 0;
int in_block = 0;

void init_array(int *array) {
	int i = 0;

	for ( i = 0; i < data_array_elements; i++ )
	{
		array[i] = i;

	}
}

int calc_sum(int *array) {
	int i = 0;
	int sum = 0;
	int first_error = 0;
	int numberOfErrors = 0;

	for ( i = 0; i < data_array_elements; i++)
	{
		sum += array[i];

		if ( array[i] != i )
		{
			numberOfErrors++;

			if (!first_error) {
				if (!in_block && robust_printing) {
					printf(" - i: %n\r\n", ind);
					printf("   E: {%i: %i,", i, array[i]);
					first_error = 1;
					in_block = 1;

				}
				else if (in_block && robust_printing){
					printf("   E: {%i: %i,", i, array[i]);
					first_error = 1;
				}
			}
			else{
				if (robust_printing)
					printf("%i: %i,", i, array[i]);
			}

			//printf("* E,%i,%i\r\n", i, array[i]);

			array[i] = i;
			local_errors++;
		}

	}

	if (first_error && robust_printing) {
		printf("}\r\n");
		first_error = 0;
	}

	if (!robust_printing && numberOfErrors > 0) {
		if (!in_block) {
			printf(" - i: %n\r\n", ind);
			printf("   E: %i\r\n", numberOfErrors);
			in_block = 1;
		}
		else {
			printf("   E: %i\r\n", numberOfErrors);
		}
	}

	if (sum != sum_const) {
		//the difference between robust and not robust printing is trivial for this one, so let it go

		//only count sum errors if there have been no other errors
		if (local_errors == 0) {
			sum_errors++;
			local_errors++;

			if (!in_block) {
				printf(" - i: %n\r\n", ind);
				printf("   S: {%i: %i}\r\n", sum_const, sum);
				first_error = 1;
				in_block = 1;

			}
			else if (in_block){
				printf("   S: {%i: %i}\r\n", sum_const, sum);
				first_error = 1;
			}
		}
	}


	return sum;
}

void cache_test(void) {
	//int data_array[data_array_elements];
	int sum = 0;
	int total_errors = 0;
	int tests_with_errors = 0;

	init_array(array);

	// Read Pattern
	while (1) {

		sum = calc_sum(array);

		if (ind % 1000 == 0 && ind != 0) {
			initUART();
			printf("# %n, %i, %i, %i\r\n", ind, total_errors, tests_with_errors, sum_errors);
		}

		ind++;
		total_errors += local_errors;
		if (local_errors > 0) {
			tests_with_errors++;
		}
		local_errors = 0;
		in_block = 0;
	}
}


int main(void)
{

	initMSP430();

	printf("\r\n---\r\n");
	printf("hw: msp430f2619\r\n");
	printf("test: cache\r\n");
	printf("mit: none\r\n");
	printf("printing: %i\r\n", robust_printing);
	printf("Array size: %i\r\n", data_array_elements);
	printf("ver: 0.1\r\n");
	printf("fac: LANSCE Nov 2015\r\n");
	printf("d:\r\n");

	cache_test();


}

void initMSP430() {
	//MSP430F2619 initialization code
	WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
	if (CALBC1_1MHZ==0xFF)					// If calibration constant erased
	{
		while(1);                               // do not load, trap CPU!!
	}
	DCOCTL = 0;                               // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
	DCOCTL = CALDCO_1MHZ;

	initUART();
}

/**
 * Initializes the UART for 9600 baud with a RX interrupt
 **/
void initUART(void) {

	P3SEL = 0x30;                             // P3.4,5 = USCI_A0 TXD/RXD
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 104;                            // 1MHz 9600; (104)decimal = 0x068h
	UCA0BR1 = 0;                              // 1MHz 9600
	UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	//IE2 |= UCA0RXIE; 						  // Enable USCI_A0 RX interrupt
}

/**
 * puts() is used by printf() to display or send a string.. This function
 * determines where printf prints to. For this case it sends a string
 * out over UART, another option could be to display the string on an
 * LCD display.
 **/
void puts(char *s) {
	char c;

	// Loops through each character in string 's'
	while (c = *s++) {
		sendByte(c);
	}
}
/**
 * puts() is used by printf() to display or send a character. This function
 * determines where printf prints to. For this case it sends a character
 * out over UART.
 **/
void putc(char b) {
	sendByte(b);
}

/**
 * Sends a single byte out through UART
 **/
void sendByte(char byte )
{
	while (!(IFG2&UCA0TXIFG)); // USCI_A0 TX buffer ready?
	UCA0TXBUF = byte; // TX -> RXed character
}

//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{

	while (!(IFG2&UCA0TXIFG));                // USCI_A0 TX buffer ready?
	UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character
}
