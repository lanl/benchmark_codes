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
// CONTACT INFO:  hquinn@lanl.gov
// LAST EDITED: 12/21/15
//
// main.c
//
// This test is a simple program for calculating matrix multiplies.  Right now
// it takes approximately the same amount of memory as the cache test. 
//
// This software is otimized for the MSP430F2619.
//
// The output is designed to go out the UART at a speed of 9,600 baud and uses a tiny
// print to reduce the printf footprint.  The tiny printf can be downloaded from 
// http://www.43oh.com/forum/viewtopic.php?f=10&t=1732  All of the output is YAML
// parsable.
//
// The input is currently using random numbers that change values every few seconds 
// in a repeatable pattern.
//
//*****************************************************************************



#include <msp430.h>
#include <string.h>
#include <stdlib.h>

#define		side				12
#define		robust_printing			1
#define		change_rate			500


int first_matrix[side][side];
int second_matrix[side][side];
unsigned long results_matrix[side][side];
unsigned long golden_matrix[side][side];

unsigned long int ind = 0;
int local_errors = 0;
int in_block = 0;
int seed_value = -1;

void sendByte(char);
void printf(char *, ...);
void initUART(void);
void initMSP430();

void init_matrices() {
  int i = 0;
  int j = 0;

  //seed the random number generator
  //the method is designed to reset SEUs in the matrices, using the current seed value
  //that way each test starts error free
  if (seed_value == -1) {
    srand(ind);
    seed_value = ind;
  }
  else {
    srand(seed_value);
  }

  //fill the matrices
  for ( i = 0; i < side; i++ ){
    for (j = 0; j < side; j++) {
      first_matrix[i][j] = rand();
      second_matrix[i][j] = rand();
    }
  }
}

void matrix_multiply(int f_matrix[][side], int s_matrix[][side], unsigned long r_matrix[][side]) {
  int i = 0;
  int j = 0;
  int k = 0;
  unsigned long sum = 0;
  
  //MM
  for ( i = 0 ; i < side ; i++ ) {
    for ( j = 0 ; j < side ; j++ ) {
      for ( k = 0 ; k < side ; k++ ) {
	sum = sum + f_matrix[i][k]*s_matrix[k][j];
      }
      
      r_matrix[i][j] = sum;
      sum = 0;
    }
  }
}

int checker(unsigned long golden_matrix[][side], unsigned long results_matrix[][side]) {
  int first_error = 0;
  int num_of_errors = 0;
  int i = 0;
  int j = 0;

  for(i=0; i<side; i++) {
    for (j = 0; j < side; j++) {
      if (golden_matrix[i][j] != results_matrix[i][j]) {
	//checker found an error, print results to screen
	if (!first_error) {
	  if (!in_block && robust_printing) {
	    printf(" - i: %n\r\n", ind);
	    printf("   E: {%i_%i: [%x, %x],", i, j, golden_matrix[i][j], results_matrix[i][j]);
	    first_error = 1;
	    in_block = 1;
	  }
	  else if (in_block && robust_printing){
	    printf("   E: {%i_%i: [%x, %x],", i, j, golden_matrix[i][j], results_matrix[i][j]);
	    first_error = 1;
	  }
	}
	else {
	  if (robust_printing)
	    printf("%i_%i: [%x, %x],", i, j, golden_matrix[i][j], results_matrix[i][j]);
	  
	}
	num_of_errors++;
      }
    }
  }
  
  if (first_error) {
    printf("}\r\n");
    first_error = 0;
  }
  
  if (!robust_printing && (num_of_errors > 0)) {
    if (!in_block) {
      printf(" - i: %n\r\n", ind);
      printf("   E: %i\r\n", num_of_errors);
      in_block = 1;
    }
    else {
      printf("   E: %i\r\n", num_of_errors);
    }
  }
  
  return num_of_errors;
}

void matrix_multiply_test() {
  
  //initialize variables
  int total_errors = 0;
  
  init_matrices();
  //setup golden values
  matrix_multiply(first_matrix, second_matrix, golden_matrix);
  
  while (1) {
    matrix_multiply(first_matrix, second_matrix, results_matrix);
    local_errors = checker(golden_matrix, results_matrix);
    
    //if there is an error, fix the input matrics
    //golden is recomputed so that the code doesn't
    //have to figure out if the error was in the results
    //or golden matrix
    if (local_errors > 0) {
      init_matrices();
      matrix_multiply(first_matrix, second_matrix, golden_matrix);
    }
    
    //acking to see if alive, as well as changing input values
    if (ind % change_rate == 0) {
      printf("# %n, %i\r\n", ind, total_errors);
      seed_value = -1;
      init_matrices();
      //have to recompute the golden
      matrix_multiply(first_matrix, second_matrix, golden_matrix);
    }
    
    //reset vars and such
    ind++;
    total_errors += local_errors;
    local_errors = 0;
    in_block = 0;
  }

}

int main()
{
  //initalize the part
  initMSP430();

  //print the YAML header
  printf("\r\n---\r\n");
  printf("hw: msp430f2619\r\n");
  printf("test: MM\r\n");
  printf("mit: none\r\n");
  printf("printing: %i\r\n", robust_printing);
  printf("input change rate: %i\r\n", change_rate);
  printf("Side matrix size: %i\r\n", side);
  printf("ver: 0.1\r\n");
  printf("fac: LANSCE Nov 2015\r\n");
  printf("d:\r\n");

  //start test
  matrix_multiply_test();
  
  return 0;

}

void initMSP430() {
  //MSP430F2619 initialization code
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  if (CALBC1_1MHZ==0xFF)		    // If calibration constant erased
    {
      while(1);                             // do not load, trap CPU!!
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
