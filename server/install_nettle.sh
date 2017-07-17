#!/bin/sh

#
## simple Script can be enhanced
#

#
## Dependancies :
##
##	cunit >= 2.1
#

cd ./.libs/nettle-3.3
./configure
make
make check
#
## if make install does not run successfully
## you can still run the program using the command
## make static
#
make install
