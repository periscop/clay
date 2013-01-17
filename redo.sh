#!/bin/sh
make maintainer-clean
./get_submodules.sh
./autogen.sh
./configure --prefix=$HOME/usr \
            --with-osl=system --with-osl-prefix=$HOME/usr \
            --with-clan=system --with-clan-prefix=$HOME/usr \
            --with-cloog=system --with-cloog-prefix=$HOME/usr \
            --with-candl=system --with-candl-prefix=$HOME/usr 

#./configure --prefix=$HOME/usr \
#            --with-osl=bundled \
#            --with-clan=bundled \
#            --with-cloog=bundled \
#            --with-candl=bundled 

make 

