//*****************************************************************************
//
// AUTHOR:  Heather Quinn, Krishna Gnawali
// CONTACT INFO:  hquinn at lanl dot gov
// LAST EDITED: 12/09/2022
//
// main.c
//
// This test is a simple program for instrumenting SRAM memory blocks or caches.
// This version of the cache test follows the march test methodology used for reliability
// testing for filling the memory array.  There are four different memory patterns in the
// pattern file: 0x0, 0xFFFF, 0x5555, 0xAAAA.  Unlike the other benchmarks, the  memory
// array can change size or values with only a simple change to the number of data_array_elements.
// The pattern is hard coded into the test.  
//
// The test code now also does double reads on error to check to see if there is an error
// in address decoding for the array under test or the pattern.  For the MSP430F2619 these
// double reads help determine whether the problem is in flash or static memory.
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
#include "stdio.h"

void printHeader(void);
void sendByte(char);
void initUART(void);
void initMSP430();

#define     robust_printing         1
#define     data_array_elements     848

int array[data_array_elements];

unsigned long int ind = 0;
int local_errors = 0;
int sum_errors = 0;
int in_block = 0;

void init_array(int *array)
{
    int i = 0;

    for (i = 0; i < data_array_elements; i++)
    {
        array[i] = 0;

    }
}

int check_array(int *array)
{
    int i = 0;
    int first_error = 0;
    int numberOfErrors = 0;

    for (i = 0; i < data_array_elements; i++)
        {

            int sarr_val1 = array[i];
            int sarr_val2 = array[i];
            if (sarr_val1 != 0 || sarr_val2 != 0) //there is either an SEU or SET
            {
                numberOfErrors++;
                if (sarr_val1 == sarr_val2) //second read doesn't match either
                {
                    printf(" - i: %u\r\n", ind);
                    printf("   SARR_SEU: {%i: %i}\n\r", sarr_val1, sarr_val2);
                }
                else
                {
                    printf(" - i: %u\r\n", ind);
                    printf("   SET: {%i: %i}\n\r", sarr_val1, sarr_val2);
                }
            }
            array[i] = 0xFFFF;
        }

        for (i = data_array_elements - 1; i > -1; i--)
        {

            int sarr_val1 = array[i];
            int sarr_val2 = array[i];
            if (sarr_val1 != 0xFFFF || sarr_val2 != 0xFFFF) //there is either an SEU or SET
            {
                numberOfErrors++;
                if (sarr_val1 == sarr_val2) //second read doesn't match either
                {
                    printf(" - i: %u\r\n", ind);
                    printf("   SARR_SEU: {%i: %i}\n\r", sarr_val1, sarr_val2);
                }
                else
                {
                    printf(" - i: %u\r\n", ind);
                    printf("   SET: {%i: %i}\n\r", sarr_val1, sarr_val2);
                }
            }
            array[i] = 0xAAAA;
        }
        for (i = 0; i < data_array_elements; i++)
        {

            int sarr_val1 = array[i];
            int sarr_val2 = array[i];
            if (sarr_val1 != 0xAAAA || sarr_val2 != 0xAAAA) //there is either an SEU or SET
            {
                numberOfErrors++;
                if (sarr_val1 == sarr_val2) //second read doesn't match either
                {
                    printf(" - i: %u\r\n", ind);
                    printf("   SARR_SEU: {%i: %i}\n\r", sarr_val1, sarr_val2);
                }
                else
                {
                    printf(" - i: %u\r\n", ind);
                    printf("   SET: {%i: %i}\n\r", sarr_val1, sarr_val2);
                }
            }
            array[i] = 0x5555;
        }

        for (i = data_array_elements - 1; i > -1; i--)
        {

            int sarr_val1 = array[i];
            int sarr_val2 = array[i];
            if (sarr_val1 != 0x5555 || sarr_val2 != 0x5555) //there is either an SEU or SET
            {
                numberOfErrors++;
                if (sarr_val1 == sarr_val2) //second read doesn't match either
                {
                    printf(" - i: %u\r\n", ind);
                    printf("   SARR_SEU: {%i: %i}\n\r", sarr_val1, sarr_val2);
                }
                else
                {
                    printf(" - i: %u\r\n", ind);
                    printf("   SET: {%i: %i}\n\r", sarr_val1, sarr_val2);
                }
            }
            array[i] = 0x0;
        }

    return 1;

}

void cache_test(void)
{

    int sum = 0;
    int total_errors = 0;
    int tests_with_errors = 0;

    init_array(array);

// Read Pattern
    while (1)
    {

        sum = check_array(array);

        if (ind % 60 == 0)
        {
            initUART();
            printf("# %u, %i, %i, %i\r\n", ind, total_errors, tests_with_errors,
                   sum_errors);
        }

        ind++;
        total_errors += local_errors;
        if (local_errors > 0)
        {
            tests_with_errors++;
        }
        local_errors = 0;
        in_block = 0;
    }
}

int main(void)
{

    initMSP430();

    printf("\n\r---\n\r");
    printf("hw: MSP430F2619\r\n");
    printf("test: cache_march\r\n");
    printf("mit: none\r\n");
    printf("printing: %i\r\n", robust_printing);
    printf("Array size: %i\r\n", data_array_elements);
    printf("ver: 1.0\r\n");
    printf("fac: LANSCE Oct 2019\r\n");
    printf("d:\r\n");

    cache_test();
}

void initMSP430()
{
//MSP430F2619 initialization code
    WDTCTL = WDTPW + WDTHOLD;

    if (CALBC1_1MHZ == 0xFF)				// If calibration constant erased
    {
        while (1)
            ;                               // do not load, trap CPU!!
    }
    DCOCTL = 0;                          // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
    DCOCTL = CALDCO_1MHZ;

    initUART();
}

/**
 * Initializes the UART for 9600 baud with a RX interrupt
 **/
void initUART(void)
{

    P3SEL = 0x30;                             // P3.4,5 = USCI_A0 TXD/RXD
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                           // 1MHz 9600; (104)decimal = 0x068h
    UCA0BR1 = 0;                              // 1MHz 9600
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**
//IE2 |= UCA0RXIE; 						  // Enable USCI_A0 RX interrupt
}

/**
 * puts() is used by printf() to display or send a string.. This function
 * determines where printf prints to. For this case it sends a string
 * out over UART, another option could be to display the string on an
 * LCD display.
 **/
int puts(const char *_ptr)
{
    unsigned int i, len;

    len = strlen(_ptr);

    for (i = 0; i < len; i++)
    {
        sendByte(_ptr[i]);
    }

    return len;
}
/**
 * puts() is used by printf() to display or send a character. This function
 * determines where printf prints to. For this case it sends a character
 * out over UART.
 **/
int putc(int _x, FILE *_fp)
{
    sendByte(_x);

    return _x;
}

/**
 * Sends a single byte out through UART
 **/
void sendByte(char byte)
{
    while (!(IFG2 & UCA0TXIFG))
        ; // USCI_A0 TX buffer ready?
    UCA0TXBUF = byte; // TX -> RXed character
}

//  Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{

    while (!(IFG2 & UCA0TXIFG))
        ;                // USCI_A0 TX buffer ready?
    UCA0TXBUF = UCA0RXBUF;                    // TX -> RXed character
}


