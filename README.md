# benchmark_codes

The Mitigation Working Group Micocontroller Code

Description

This code is part of the mitigation working group code base to be used
for testing microprocessors and FPGAs.  This specific code is the code
developed at Los Alamos National Laboratory for testing
microcontrollers.  This code includes a handful of codes that can be
used to test microcontrollers for accelerated radiation tests.  This
version of the code includes: AES, LANL cache test, matrix multiply
and quicksort.  We also use CoreMark for testing purposes, which can
be found here: http://www.eembc.org/coremark/index.php.

The ver 1.0 update of benchmarks has made several changes to the cache
test, the quicksort codes.  The cache test improvements help
distinguish the difference between address decoder erros and SEUs.
The quicksort changes allow us to release the code with the software,
unlike the 0.1 version.  Several inputs have been changed for the
cache, quicksort, and matrix multiply tests.  These improvements help
determine the effect of the inputs on the errors.

Input

The inputs have been changed on several of the codes in the 1.0 update
that puvished on 6/14/2023.

Output

All of the tests output YAML parsable text on the part's UART.  It is
possible to analyze this output with pyYAML.  LANL has codes, but they
are not covered under this license.  We plan to release them in the
future.  

The outputs are also designed to be "robust" and "non-robust" for
different types of testing.  Robust printing prints all of the
information about an error, including the test number and all of the
(correct, incorrect) value pairs.  Non-robust printing only prints the
test number and the number of errors.  For a test like quicksort, the
amount of output that can be generate could be timeconsuming, so
non-robust printing is needed when there are many errors.  It is
important that the printing not take over the computation process, so
that the real computation is the test and not printf.  For radiation
tests where the flux is very low, such as at LANSCE, it is possible to
use the robust printing.  For radiation tests were the flux is very
high, such as heavy ion testing, it is better to use non-robust
printing.

Installation

This code is very bare boned.  It works with Code Composer Studio, but
projects are not given.  The user will need to import the code into their
own Code Composer Studio projects.  Also, the code is designed to work with
the MSP430F2619.  The initialization and putc codes will need to be 
modified to work with other microcontrollers.

Copyright and license

Los Alamos National Security, LLC (LANS) owns the copyright to this
portion of the mitigation working group's software codes.  This
particular set of codes is for microcontrollers, which it identifies
internally as LA-CC-15-052. The license is BSD-ish with a
"modifications must be indicated" clause. See LICENSE.md for the full
text.

Contact

Heather Quinn, hquinn at lanl dot gov
