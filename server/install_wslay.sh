#!/bin/sh

#
## simple Script can be enhanced
#

#
## Dependancies :
##
##	cunit >= 2.1
##	nettle >= 2.4
##	python-sphinx for man generation
#

cd ./wslay/
autoreconf -i
automake
autoconf
./configure
make
