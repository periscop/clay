#!/bin/sh
autoreconf -i 
if test -f osl/autogen.sh; then
	(cd osl; ./autogen.sh)
fi
if test -f clan/autogen.sh; then
	(cd clan; ./autogen.sh)
fi
if test -f cloog/autogen.sh; then
	(cd cloog; ./autogen.sh)
fi
if test -f candl/autogen.sh; then
	(cd candl; ./autogen.sh)
fi
