#!/bin/sh
make maintainer-clean
#./get_submodules.sh
./autogen.sh
#./configure --prefix=/home/jpoudroux/dev/pocc/usr --with-osl=system --with-osl-prefix=/home/jpoudroux/dev/pocc/usr
./configure --prefix=$HOME/usr --with-osl=system --with-osl-prefix=$HOME/usr
#./configure --prefix=$HOME/usr
make
