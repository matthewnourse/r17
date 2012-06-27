# r17 #

A relational concurrent data mining language.
More info, documentation and binaries for some platforms:
[http://www.rseventeen.com/](http://www.rseventeen.com/)

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


## Running the r17 unit tests ##
If you want to run all the r17 unit tests, you need:
* automatic SSH access to the local machine beacuse some of the tests try to SSH to localhost as the current user.
* a local WebDAV server.  I've only tested with Apache's mod_dav.
* an empty `[WebDAV document root]/np1_test_reliable_storage_remote_root` directory.

These prerequisites are not required to run r17 normally unless you want to use the relevant distributed-computation styles.


### Apache configuration for the unit tests ###
`# Load the WebDAV modules`  
`LoadModule dav_module /usr/lib/apache2/modules/mod_dav.so`  
`LoadModule dav_fs_module /usr/lib/apache2/modules/mod_dav_fs.so`  
`DAVLockDB ${APACHE_LOCK_DIR}/DAVLock`  
` `  
`# Put this in the default VirtualHost.`  
`<Directory /var/www/np1_test_reliable_storage_remote_root>`  
`  Dav On`  
`</Directory>`  

### How to run the tests ###
`make check`

### What if the tests don't pass for me? ###
The tests are known to pass on Debian Linux, Ubuntu Linux and Mac OS X.  FreeBSD is tested less often.
Please report test failures using the email address in the failure report, or by using the details at
[http://www.rseventeen.com/contact/](http://www.rseventeen.com/contact/).
