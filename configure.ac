AC_INIT([renderer], [0.1])
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([src/main.cpp])

AC_PROG_CXX

PKG_CHECK_MODULES([CAIRO], [pangocairo])
PKG_CHECK_MODULES([GFLAGS], [gflags])
PKG_CHECK_MODULES([PROTOBUF], [protobuf])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])

AC_CHECK_PROG([PROTOC], [protoc], [protoc])

AC_OUTPUT
