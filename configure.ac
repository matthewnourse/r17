AC_INIT([r17], [1.5.1], [bug-report@rseventeen.com], [r17], [http://www.rseventeen.com/])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADERS([config.h])
: ${CXXFLAGS=-Wall -Werror -Wfatal-errors -fno-strict-aliasing -Wstrict-aliasing -std=c++0x -fno-rtti -fno-exceptions -fno-threadsafe-statics -O3}
AC_PROG_CXX

: ${LIBS="-lz $LIBS"}
AC_CHECK_LIB(
    [z],
    [gzbuffer],
    [AC_MSG_NOTICE([Found zlib with gzbuffer, will get better performance])],
    [AC_CHECK_LIB(
        [z],
        [gzdopen],
        [AC_MSG_NOTICE([Found zlib WITHOUT gzbuffer, performance will suffer.]) AC_DEFINE([HAVE_NO_GZBUFFER], [1], [The zlib version is too old to have the gzbuffer function.])],
        [AC_MSG_FAILURE([zlib not found])])])

AC_CHECK_LIB([curl], [curl_easy_init], [LIBS="-lcurl $LIBS"], [AC_MSG_FAILURE([libcurl not found])])

: ${LIBS="-lpcre $LIBS"}
AC_CHECK_LIB([pcre], [pcre_study], [LIBS="-lpcre $LIBS"], [AC_MSG_FAILURE([libpcre not found])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
