#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([net_prog], [1.0], [forfolias@linuxteam.cs.teilar.gr])
AM_INIT_AUTOMAKE([net_prog], [1.0])
AC_CONFIG_SRCDIR(.)
AC_CONFIG_FILES([Makefile] [data/Makefile])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h limits.h pthread.h signal.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT16_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([getcwd gethostbyname inet_ntoa memset mkdir socket])

AC_OUTPUT
