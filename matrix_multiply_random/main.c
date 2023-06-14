//*****************************************************************************
//
// AUTHOR:  Heather Quinn
// CONTACT INFO:  hquinn@lanl.gov
// LAST EDITED: 12/21/15
//
// main.c
//
// This test is a simple program for calculating matrix multiplies.  Right now
// it takes approximately the same amount of memory as the cache test.  Like the
// cache static test, this test also includes a pattern file for ease of checking.
// The pattern includes random values that are used to mimic real user data.  To
// change the size of the matries, the pattern needs to be updated.
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
#include "stdio.h"

#include "pattern.h"

void printHeader(void);
void sendByte(char);
void initUART(void);
void initMSP430();

#define     robust_printing           1
#define     side                      12
#define     change_rate               50

int first_matrix[side][side];
int second_matrix[side][side];
unsigned long results_matrix[side][side];
//unsigned long golden_matrix[side][side];

unsigned long int ind = 0;
int local_errors = 0;
int sum_errors = 0;
int in_block = 0;

void init_matrices()
{
    int i = 0;
    int j = 0;

    srand(-1);

    //fill the matrices
    for (i = 0; i < side; i++)
    {
        for (j = 0; j < side; j++)
        {
            first_matrix[i][j] = rand();
            second_matrix[i][j] = rand();
        }
    }
}

void matrix_multiply(int f_matrix[][side], int s_matrix[][side],
                     unsigned long r_matrix[][side])
{
    int i = 0;
    int j = 0;
    int k = 0;
    unsigned long sum = 0;

    //MM
    for (i = 0; i < side; i++)
    {
        for (j = 0; j < side; j++)
        {
            for (k = 0; k < side; k++)
            {
                sum = sum + f_matrix[i][k] * s_matrix[k][j];
            }

            r_matrix[i][j] = sum;
            sum = 0;
        }
    }

}

int checker(unsigned long golden_matrix[][side],
            unsigned long results_matrix[][side])
{
    int first_error = 0;
    int num_of_errors = 0;
    int i = 0;
    int j = 0;

    for (i = 0; i < side; i++)
    {
        for (j = 0; j < side; j++)
        {

           if (golden_matrix[i][j] != results_matrix[i][j])
            {
                if (!first_error)
                {
                    if (!in_block && robust_printing)
                    {
                        printf(" - i: %u\r\n", ind);
                        printf("   E: {%i_%i: [%x, %x],", i, j,
                               golden_matrix[i][j], results_matrix[i][j]);
                        first_error = 1;
                        in_block = 1;
                    }
                    else if (in_block && robust_printing)
                    {
                        printf("   E: {%i_%i: [%x, %x],", i, j,
                               golden_matrix[i][j], results_matrix[i][j]);
                        first_error = 1;
                    }
                }
                else
                {
                    if (robust_printing)
                        printf("%i_%i: [%x, %x],", i, j, golden_matrix[i][j],
                               results_matrix[i][j]);

                }
                num_of_errors++;
            }
        }
    }

    if (first_error)
    {
        printf("}\r\n");
        first_error = 0;
    }

    if (!robust_printing && (num_of_errors > 0))
    {
        if (!in_block)
        {
            printf(" - i: %u\r\n", ind);
            printf("   E: %i\r\n", num_of_errors);
            in_block = 1;
        }
        else
        {
            printf("   E: %i\r\n", num_of_errors);
        }
    }

    return num_of_errors;
}

void matrix_multiply_test()
{

    //initialize variables
    int total_errors = 0;
    int i = 0;
    int j = 0;

    init_matrices();

    //the commented out code will print out the pattern, if you need to update it.
    //matrix_multiply(first_matrix, second_matrix, golden_matrix);

    /*printf("const unsigned long golden_matrix[][] = {\r\n");
    for (i = 0; i < side; i++)
    {
        printf("{");
        for (j = 0; j < side; j++)
        {
            printf("%n, ", golden_matrix[i][j]);
        }
        printf("},\r\n");
    }
    printf("\r\n};\r\n");*/

    while (1)
    {
        matrix_multiply(first_matrix, second_matrix, results_matrix);
        local_errors = checker(golden_matrix, results_matrix);

        if (local_errors > 0)
        {
            init_matrices();
            //matrix_multiply(first_matrix, second_matrix, golden_matrix);
        }

        if (ind % change_rate == 0)
        {
            if (ind != 0)
            {
                initUART();
            }

            printf("# %u, %i\r\n", ind, total_errors);
        }

        //reset vars and such
        ind++;
        total_errors += local_errors;
        local_errors = 0;
        in_block = 0;
    }

}

int main(void)
{

    initMSP430();

    printf("\n\r---\n\r");
    printf("hw: MSP430F2619\r\n");
    printf("test: MM_rand_flash\r\n");
    printf("mit: none\r\n");
    printf("printing: %i\r\n", robust_printing);
    printf("Side matrix size: %i\r\n", side);
    printf("ver: 1.0\r\n");
    printf("fac: LANSCE Oct 2019\r\n");
    printf("d:\r\n");

    matrix_multiply_test();
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

