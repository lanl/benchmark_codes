/*
 *****************************************************************************
//
// AUTHOR:  Heather Quinn
// CONTACT INFO:  hquinn at lanl dot gov
// LAST EDITED: 12/21/15
//
// tiva_qsort.c
//
// This test is a simple program for testing quicksort.  The input data are
// the same pattern as the cache_static tests: mostly 0s, mostly 1s, mostly
// As, and msotly 5s.  As with the other test, the values have their addresses
// encoded to see if the issue could be address decoding.  As with the cache_static
// test, larger test arrays need to have the pattern changed.  The test is
// designed to test whether sorting many numbers cause more errors than sorting
// an already sorted array, so the algorithm switches between sorting forward twice
// and then sorting backward twice.  This version of the code includes qsort algorithms,
// unlike the previous version.  The forward and reverse sorts were created by
// another engineer using a clean-room technique to recreate the quicksort algorithms.
// without violating the licensing of any of the existing quicksort algorithms.  The
// new algorithm is functionally the same.
//
// This software is otimized for microcontrollers.  In particular, it was designed
// for the Texas Instruments MSP430F2619.

// The output is designed to go out the UART at a speed of 9,600 baud and uses a tiny
// print to reduce the printf footprint.  The tiny printf can be downloaded from 
// http://www.43oh.com/forum/viewtopic.php?f=10&t=1732  All of the output is YAML
// parsable.
//
 *****************************************************************************/

#include <msp430.h>
#include <string.h>
#include "stdio.h"

#include "pattern.h"

void printHeader(void);
void sendByte(char);
void initUART(void);
void initMSP430();

#define     robust_printing           1
#define     array_elements            180
#define     change_rate               50

int seed_value = -1;
int array[array_elements];

unsigned long int ind = 0;
int local_errors = 0;
int sum_errors = 0;
int in_block = 0;

void init_array()
{
    int i = 0;

    for (i = 0; i < array_elements; i++)
    {
        array[i] = static_pattern[i];
    }
}

void quick_sort (int *a, int n) {
    if (n < 2)
        return;
    int p = a[n / 2];
    int *l = a;
    int *r = a + n - 1;
    while (l <= r) {
        if (*l < p) {
            l++;
        }
        else if (*r > p) {
            r--;
        }
        else {
            int t = *l;
            *l = *r;
            *r = t;
            l++;
            r--;
        }
    }
    quick_sort(a, r - a + 1);
    quick_sort(l, a + n - l);
}

void quick_sort_rev (int *a, int n) {
    if (n < 2)
        return;
    int p = a[n / 2];
    int *l = a;
    int *r = a + n - 1;
    while (l <= r) {
        if (*l > p) {
            l++;
        }
        else if (*r < p) {
            r--;
        }
        else {
            int t = *l;
            *l = *r;
            *r = t;
            l++;
            r--;
        }
    }
    quick_sort_rev(a, r - a + 1);
    quick_sort_rev(l, a + n - l);
}

int checker(int golden_array[], int dut_array[], int sub_test) {
    int first_error = 0;
    int num_of_errors = 0;
    int i = 0;

    for(i=0; i<array_elements; i++) {
        if (golden_array[i] != dut_array[i]) {
            if (!first_error) {
                if (!in_block && robust_printing) {
                    printf(" - i: %u, %i\r\n", ind, sub_test);
                    printf("   E: {%i: [%x, %x],", i, golden_array[i], dut_array[i]);
                    first_error = 1;
                    in_block = 1;
                }
                else if (in_block && robust_printing){
                    printf("   E: {%i: [%x, %x],", i, golden_array[i], dut_array[i]);
                    first_error = 1;
                }
            }
            else {
                if (robust_printing)
                    printf("%i: [%x, %x],", i, golden_array[i], dut_array[i]);

            }
            num_of_errors++;
        }
    }

    if (first_error) {
        printf("}\r\n");
        first_error = 0;
    }

    if (!robust_printing && (num_of_errors > 0)) {
        if (!in_block) {
            printf(" - i: %u, %i\r\n", ind, sub_test);
            printf("   E: %i\r\n", num_of_errors);
            in_block = 1;
        }
        else {
            printf("   E: %i\r\n", num_of_errors);
        }
    }

    return num_of_errors;
}

void qsort_test() {

    //initialize variables
    int total_errors = 0;
    int n = sizeof array / sizeof array[0];
    int i = 0;

    init_array();

    while (1) {
        for (i = 0; i < 4; i++) {
            if (i < 2) {
                quick_sort(array, n);
                local_errors = checker(static_pattern_forward, array, i);
            }
            else {
                quick_sort_rev(array, n);
                local_errors = checker(static_pattern_reverse, array, i);
            }

            if (local_errors > 0) {
                init_array();
            }

            total_errors += local_errors;
            local_errors = 0;
            in_block = 0;

        }

        if (ind % change_rate == 0) {
            if (ind != 0) {
                initUART();
            }

            printf("# %u, %i\r\n", ind, total_errors);
        }

        //reset vars and such
        ind++;

    }

}

int main(void)
{

    initMSP430();

    printf("\n\r---\n\r");
    printf("hw: MSP430F2619\r\n");
    printf("test: qsort_flash\r\n");
    printf("mit: none\r\n");
    printf("printing: %i\r\n", robust_printing);
    printf("Array size: %i\r\n", array_elements);
    printf("ver: 1.0\r\n");
    printf("fac: LANSCE Oct 2019\r\n");
    printf("d:\r\n");

    qsort_test();
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

