# r17 #

A relational concurrent data mining language.
More info, documentation and binaries for some platforms:
http://www.rseventeen.com/

## What you need to compile r17 ##
* A UNIX-like system such as Linux, BSD or OS X.
* g++ 4.4.x or later because r17 uses some C++0x features.  I have only
  attempted to compile r17 with g++.
* zlib (preferably 1.2.4 or later), libcurl, libpcre.


## How to compile r17 ##

### Compilation and installation for those in a hurry ###
./configure && make

The output is one executable file: r17.  Move it to somewhere in your path and enjoy.


### How to static link with zlib, libcurl and PCRE ###
This is particularly useful for compilation & distribution on platforms which
might not have all the dependencies by default, or that have a pre-1.2.4
version of zlib.  R17 works with earlier versions of zlib but decompression
performance is significantly better with 1.2.4 and later.

zlib:
* Download the zlib source, untar and cd into the resulting directory.
* ./configure --static
* make

libcurl:
* Download the the curl source, untar and cd into the resulting directory.
* ./configure --disable-shared --enable-static --disable-ares --disable-ldap
  --disable-crypto-auth --disable-tls-srp --without-ssl
  --with-zlib=`[zlib source dir]`
  --without-gnutls --without-polarssl --without-nss --without-axtls
  --without-ca-bundle --without-ca-path --without-libssh2 --without-librtmp
  --without-libidn
* make

PCRE:
* Download the PCRE source, untar and cd into the resulting directory.
* ./configure --disable-shared --enable-static --disable-cpp --enable-utf8
* make

Then:
* cd back into the r17 directory.
* ./configure --with-zlib=`[zlib source dir]`
  --with-libcurl=`[libcurl source dir]`
  --with-libpcre=`[libpcre source dir]`
* make

As for the default configuration, the only output is the r17 executable.

