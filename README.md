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
