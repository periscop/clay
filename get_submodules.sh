#!/bin/sh
git submodule init
git submodule update
if test -f osl/get_submodules.sh; then
	(cd osl; ./get_submodules.sh)
fi
if test -f clan/get_submodules.sh; then
	(cd clan; ./get_submodules.sh)
fi
if test -f cloog/get_submodules.sh; then
	(cd cloog; ./get_submodules.sh)
fi
if test -f candl/get_submodules.sh; then
	(cd candl; ./get_submodules.sh)
fi
