dnl $Id$
dnl config.m4 for extension snap

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(snap, for snap support,
dnl Make sure that the comment is aligned:
dnl [  --with-snap             Include snap support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(snap, whether to enable snap support,
[  --enable-snap           Enable snap support])

if test "$PHP_SNAP" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-snap -> check with-path
  dnl SEARCH_PATH="/usr/local /usr /usr/home/litton/snap/tests/"     # you might want to change this
  dnl SEARCH_FOR="/include/snapper.h"  # you most likely want to change this
  dnl if test -r $PHP_SNAP/$SEARCH_FOR; then # path given as parameter
  dnl   SNAP_DIR=$PHP_SNAP
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for snap files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       SNAP_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$SNAP_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the snap distribution])
  dnl fi

  dnl # --with-snap -> add include path
  dnl PHP_ADD_INCLUDE($SNAP_DIR/include)

  dnl # --with-snap -> check for lib and symbol presence
  dnl LIBNAME=snap # you may want to change this
  dnl LIBSYMBOL=snap # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $SNAP_DIR/$PHP_LIBDIR, SNAP_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_SNAPLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong snap lib version or lib not found])
  dnl ],[
  dnl   -L$SNAP_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(SNAP_SHARED_LIBADD)

  PHP_NEW_EXTENSION(snap, snap.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
