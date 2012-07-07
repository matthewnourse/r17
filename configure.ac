AC_INIT([r17], [1.6.0], [bug-report@rseventeen.com], [r17], [http://www.rseventeen.com/])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADERS([config.h])
: ${CXXFLAGS=-Wall -Werror -Wfatal-errors -fno-strict-aliasing -Wstrict-aliasing -std=c++0x -fno-rtti -fno-exceptions -fno-threadsafe-statics -O3}
AC_PROG_CXX

# Can either statically link with libz and specify the zlib source dir, or dynamically link using the systemwide libz.
zlib_lib="-lz"
zlib_inc=

AC_ARG_WITH(
  zlib,
  [  --with-zlib=PATH   prefix for zlib source dir. Use only if you want to statically link with zlib],
  [ zlib_lib="$with_zlib/libz.a"
    zlib_inc="-I$with_zlib"
    AC_MSG_NOTICE([Using zlib in $with_zlib])],
  [AC_CHECK_LIB(
    [z],
    [gzbuffer],
    [AC_MSG_NOTICE([Found zlib with gzbuffer, will get better performance])],
    [AC_CHECK_LIB(
        [z],
        [gzdopen],
        [AC_MSG_NOTICE([Found zlib WITHOUT gzbuffer, performance will suffer.]) AC_DEFINE([HAVE_NO_GZBUFFER], [1], [The zlib version is too old to have the gzbuffer function.])],
        [AC_MSG_FAILURE([zlib not found])])])]
) 



# Can either statically link with libcurl and specify the source dir, or dynamically link using the systemwide
# libcurl.
libcurl_lib="-lcurl"
libcurl_inc=

AC_ARG_WITH(
  libcurl,
  [  --with-libcurl=PATH   prefix for libcurl source dir. Use only if you want to statically link with libcurl],
  [ libcurl_lib="$with_libcurl/lib/.libs/libcurl.a"
    uname_os=`uname`
    if test "$uname_os" = "Linux"; then
      libcurl_lib="$libcurl_lib -lrt"
    fi
    libcurl_inc="-I$with_libcurl/include"
    AC_MSG_NOTICE([Using libcurl in $with_libcurl])],
  [AC_CHECK_LIB([curl], [curl_easy_init], [], [AC_MSG_FAILURE([libcurl not found])])]
)


# Can either statically link with libpcre and specify the source dir, or dynamically link using the systemwide
# libpcre.
libpcre_lib="-lpcre"
libpcre_inc=

AC_ARG_WITH(
  libpcre,
  [  --with-libpcre=PATH   prefix for libpcre source dir. Use only if you want to statically link with libpcre],
  [ libpcre_lib="$with_libpcre/.libs/libpcre.a"
    libpcre_inc="-I$with_libpcre"
    AC_MSG_NOTICE([Using libpcre in $with_libpcre])],
  [AC_CHECK_LIB([pcre], [pcre_study], [], [AC_MSG_FAILURE([libpcre not found])])]
)

LIBS="$zlib_lib $libcurl_lib $libpcre_lib $LIBS"
CXXFLAGS="$zlib_inc $libcurl_inc $libpcre_inc $CXXFLAGS"

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
