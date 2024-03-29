dnl GNUPG_CHECK_TYPEDEF(TYPE, HAVE_NAME [, COMMENT])
dnl Check whether a typedef exists and create a #define $2 if it exists
dnl Copyright (C) 2003 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
dnl
AC_DEFUN([GNUPG_CHECK_TYPEDEF],
  [ AC_MSG_CHECKING(for $1 typedef)
    AC_CACHE_VAL(gnupg_cv_typedef_$1,
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <stdlib.h>
    #include <sys/types.h>]], [[
    #undef $1
    int a = sizeof($1);
    ]])],[gnupg_cv_typedef_$1=yes],[gnupg_cv_typedef_$1=no ])])
    AC_MSG_RESULT($gnupg_cv_typedef_$1)
    if test "$gnupg_cv_typedef_$1" = yes; then
        AC_DEFINE($2, 1, $3)
    fi
  ])

