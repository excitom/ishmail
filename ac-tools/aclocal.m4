dnl ACX_SAVE_STATE/ACX_RESTORE_STATE
dnl Save/restore flags
dnl 
dnl ACX_SAVE_STATE
AC_DEFUN(ACX_SAVE_STATE,
[
    save_CFLAGS=$CFLAGS
    save_CXXFLAGS=$CXXFLAGS
    save_CPPFLAGS=$CPPFLAGS
    save_LDFLAGS=$LDFLAGS
    save_LIBS=$LIBS
])
dnl ACX_RESTORE_STATE
AC_DEFUN(ACX_RESTORE_STATE,
[
    CFLAGS=$save_CFLAGS
    CXXFLAGS=$save_CXXFLAGS
    CPPFLAGS=$save_CPPFLAGS
    LDFLAGS=$save_LDFLAGS
    LIBS=$save_LIBS
])


AC_DEFUN(ACX_CHECK_CC_FLAGS,
[
AC_REQUIRE([AC_PROG_CC])
AC_CACHE_CHECK(whether ${CC-cc} accepts $1, ac_$2,
[echo 'void f(){}' > conftest.c
if test -z "`${CC-cc} $1 -c conftest.c 2>&1`"; then
	ac_$2=yes
else
	ac_$2=no
fi
rm -f conftest*
])
if test "$ac_$2" = yes; then
	:
	$3
else
	:
	$4
fi
])

dnl **** Check for gcc strength-reduce bug ****
AC_DEFUN(ACX_GCC_STRENGTH_REDUCE,
[
AC_REQUIRE([AC_PROG_CC])
AC_CACHE_CHECK( "for gcc strength-reduce bug", ac_cv_c_gcc_strength_bug,
  AC_TRY_RUN([
    int main(void) {
      static int Array[[3]];
      unsigned int B = 3;
      int i;
      for(i=0; i<B; i++) Array[[i]] = i - 3;
      exit( Array[[1]] != -2 );
    }
    ],

    ac_cv_c_gcc_strength_bug="no",
    ac_cv_c_gcc_strength_bug="yes",
    ac_cv_c_gcc_strength_bug="yes")
  )
  
  if test "$ac_cv_c_gcc_strength_bug" = "yes"
  then
    :
    $1
  else
    :
    $2
  fi
])


dnl ICE_CHECK_DECL (FUNCTION, HEADER-FILE...)
dnl -----------------------------------------
dnl
dnl If FUNCTION is available, define `HAVE_FUNCTION'.  If it is declared
dnl in one of the headers named in the whitespace-separated list
dnl HEADER_FILE, define `HAVE_FUNCTION_DECL` (in all capitals).
dnl
AC_DEFUN(ICE_CHECK_DECL,
[
changequote(,)dnl
ice_tr=`echo $1 | tr '[a-z]' '[A-Z]'`
changequote([,])dnl
ice_have_tr=HAVE_$ice_tr
ice_have_decl_tr=${ice_have_tr}_DECL
ice_have_$1=no
AC_CHECK_FUNCS($1, ice_have_$1=yes)
if test "${ice_have_$1}" = yes; then
AC_MSG_CHECKING(for $1 declaration in $2)
AC_CACHE_VAL(ice_cv_have_$1_decl,
[
ice_cv_have_$1_decl=no
changequote(,)dnl
ice_re_params='[a-zA-Z_][a-zA-Z0-9_]*'
ice_re_word='(^|[^a-zA-Z_0-9_])'
changequote([,])dnl
for header in $2; do
# Check for ordinary declaration
AC_EGREP_HEADER([${ice_re_word}$1 *\(], $header,
	ice_cv_have_$1_decl=yes)
if test "$ice_cv_have_$1_decl" = yes; then
	break
fi
# Check for "fixed" declaration like "getpid _PARAMS((int))"
AC_EGREP_HEADER([${ice_re_word}$1 *$ice_re_params\(\(], $header,
	ice_cv_have_$1_decl=yes)
if test "$ice_cv_have_$1_decl" = yes; then
	break
fi
done
])
AC_MSG_RESULT($ice_cv_have_$1_decl)
if test "$ice_cv_have_$1_decl" = yes; then
AC_DEFINE_UNQUOTED(${ice_have_decl_tr})
fi
fi
])dnl

dnl ACX_CHECK_STATFS - stat(v)fs and related friends
dnl --------------
AC_DEFUN(ACX_CHECK_STATFS,
[
  AC_CHECK_FUNCS(statvfs)
  
  AC_MSG_CHECKING(whether stat(v)fs.f_fsid structure has __val array)
  AC_CACHE_VAL(acx_cv_have___val_in_f_fsid,
    AC_TRY_COMPILE([],
    [
#include <sys/types.h>
#ifdef HAVE_STATVFS
#  include <sys/statvfs.h>
#  define STATFS statvfs
#else
#  include <sys/vfs.h>
#  define STATFS statfs
#endif
    struct STATFS	fsbuf;
    int fsid = (int) fsbuf.f_fsid.__val[[0]];
    ], acx_cv_have___val_in_f_fsid=yes, acx_cv_have___val_in_f_fsid=no))

  AC_MSG_RESULT($acx_cv_have___val_in_f_fsid)
  if test "$acx_cv_have___val_in_f_fsid" = yes; then
    AC_DEFINE(HAVE___VAL_IN_F_FSID)
  fi
])

dnl ICE_FIND_MOTIF
dnl --------------
dnl
dnl Find Motif libraries and headers
dnl Put Motif include directory in motif_includes,
dnl put Motif library directory in motif_libraries,
dnl and add appropriate flags to X_CFLAGS and X_LIBS.
dnl
dnl
AC_DEFUN(ICE_FIND_MOTIF,
[
AC_REQUIRE([AC_PATH_XTRA])
motif_includes=
motif_libraries=
AC_ARG_WITH(motif-includes,
[  --with-motif-includes=DIR    Motif include files are in DIR],
motif_includes="$withval")
AC_ARG_WITH(motif-libraries,
[  --with-motif-libraries=DIR   Motif libraries are in DIR],
motif_libraries="$withval")
AC_MSG_CHECKING(for Motif)
#
#
# Search the include files.
#
if test "$motif_includes" = ""; then
AC_CACHE_VAL(ice_cv_motif_includes,
[
ice_motif_save_LIBS="$LIBS"
ice_motif_save_CFLAGS="$CFLAGS"
ice_motif_save_CPPFLAGS="$CPPFLAGS"
ice_motif_save_LDFLAGS="$LDFLAGS"
#
LIBS="$X_PRE_LIBS -lXm -lXt -lX11 $X_EXTRA_LIBS $LIBS"
CFLAGS="$X_CFLAGS $CFLAGS"
CPPFLAGS="$X_CFLAGS $CPPFLAGS"
LDFLAGS="$X_LIBS $LDFLAGS"
#
AC_TRY_COMPILE([#include <Xm/Xm.h>],[int a;],
[
# Xm/Xm.h is in the standard search path.
ice_cv_motif_includes=
],
[
# Xm/Xm.h is not in the standard search path.
# Locate it and put its directory in `motif_includes'
#
# /usr/include/Motif* are used on HP-UX (Motif).
# /usr/include/X11* are used on HP-UX (X and Athena).
# /usr/dt is used on Solaris (Motif).
# /usr/openwin is used on Solaris (X and Athena).
# Other directories are just guesses.
ice_cv_motif_includes=no
for dir in "$x_includes" "${prefix}/include" /usr/include /usr/local/include \
           /usr/include/Motif2.0 /usr/include/Motif1.2 /usr/include/Motif1.1 \
           /usr/include/X11R6 /usr/include/X11R5 /usr/include/X11R4 \
           /usr/dt/include /usr/openwin/include \
           /usr/dt/*/include /opt/*/include /usr/include/Motif* \
           "${prefix}"/*/include /usr/*/include /usr/local/*/include \
           "${prefix}"/include/* /usr/include/* /usr/local/include/*; do
if test -f "$dir/Xm/Xm.h"; then
ice_cv_motif_includes="$dir"
break
fi
done
])
#
LIBS="$ice_motif_save_LIBS"
CFLAGS="$ice_motif_save_CFLAGS"
CPPFLAGS="$ice_motif_save_CPPFLAGS"
LDFLAGS="$ice_motif_save_LDFLAGS"
])
motif_includes="$ice_cv_motif_includes"
fi
#
#
# Now for the libraries.
#
if test "$motif_libraries" = ""; then
AC_CACHE_VAL(ice_cv_motif_libraries,
[
ice_motif_save_LIBS="$LIBS"
ice_motif_save_CFLAGS="$CFLAGS"
ice_motif_save_CPPFLAGS="$CPPFLAGS"
ice_motif_save_LDFLAGS="$LDFLAGS"
#
LIBS="$X_PRE_LIBS -lXm -lXt -lX11 $X_EXTRA_LIBS $LIBS"
CFLAGS="$X_CFLAGS $CFLAGS"
CPPFLAGS="$X_CFLAGS $CPPFLAGS"
LDFLAGS="$X_LIBS $LDFLAGS"
#
AC_TRY_LINK([#include <Xm/Xm.h>],[XtToolkitInitialize();],
[
# libXm.a is in the standard search path.
ice_cv_motif_libraries=
],
[
# libXm.a is not in the standard search path.
# Locate it and put its directory in `motif_libraries'
#
# /usr/lib/Motif* are used on HP-UX (Motif).
# /usr/lib/X11* are used on HP-UX (X and Athena).
# /usr/dt is used on Solaris (Motif).
# /usr/lesstif is used on Linux (Lesstif).
# /usr/openwin is used on Solaris (X and Athena).
# Other directories are just guesses.
ice_cv_motif_libraries=no
for dir in "$x_libraries" "${prefix}/lib" /usr/lib /usr/local/lib \
           /usr/lib/Motif2.0 /usr/lib/Motif1.2 /usr/lib/Motif1.1 \
           /usr/lib/X11R6 /usr/lib/X11R5 /usr/lib/X11R4 /usr/lib/X11 \
           /usr/dt/lib /usr/openwin/lib \
           /usr/dt/*/lib /opt/*/lib /usr/lib/Motif* \
           /usr/lesstif*/lib /usr/lib/Lesstif* \
           "${prefix}"/*/lib /usr/*/lib /usr/local/*/lib \
           "${prefix}"/lib/* /usr/lib/* /usr/local/lib/*; do
if test -d "$dir" && test "`ls $dir/libXm.* 2> /dev/null`" != ""; then
ice_cv_motif_libraries="$dir"
break
fi
done
])
#
LIBS="$ice_motif_save_LIBS"
CFLAGS="$ice_motif_save_CFLAGS"
CPPFLAGS="$ice_motif_save_CPPFLAGS"
LDFLAGS="$ice_motif_save_LDFLAGS"
])
#
motif_libraries="$ice_cv_motif_libraries"
fi
# Add Motif definitions to X flags
#
if test "$motif_includes" != "" && test "$motif_includes" != "$x_includes" && test "$motif_includes" != "no"
then
X_CFLAGS="-I$motif_includes $X_CFLAGS"
fi
if test "$motif_libraries" != "" && test "$motif_libraries" != "$x_libraries" && test "$motif_libraries" != "no"
then
case "$X_LIBS" in
  *-R\ *) X_LIBS="-L$motif_libraries -R $motif_libraries $X_LIBS";;
  *-R*)   X_LIBS="-L$motif_libraries -R$motif_libraries $X_LIBS";;
  *)      X_LIBS="-L$motif_libraries $X_LIBS";;
esac
fi
#
#
motif_libraries_result="$motif_libraries"
motif_includes_result="$motif_includes"
test "$motif_libraries_result" = "" &&
  motif_libraries_result="in default path"
test "$motif_includes_result" = "" &&
  motif_includes_result="in default path"
test "$motif_libraries_result" = "no" &&
  motif_libraries_result="(none)"
test "$motif_includes_result" = "no" &&
  motif_includes_result="(none)"
AC_MSG_RESULT(
  [libraries $motif_libraries_result, headers $motif_includes_result])
])dnl

dnl ICE_CHECK_LESSTIF
dnl -----------------
dnl
dnl Define `HAVE_LESSTIF' if the Motif library is actually a LessTif library
dnl
AC_DEFUN(ICE_CHECK_LESSTIF,
[
AC_MSG_CHECKING(whether the Motif library is actually a LessTif library)
AC_CACHE_VAL(ice_cv_have_lesstif,
AC_EGREP_CPP(yes,
[#include <Xm/Xm.h>
#ifdef LesstifVersion
yes
#endif
], ice_cv_have_lesstif=yes, ice_cv_have_lesstif=no))
AC_MSG_RESULT($ice_cv_have_lesstif)
if test "$ice_cv_have_lesstif" = yes; then
AC_DEFINE(HAVE_LESSTIF)
fi
])dnl


dnl ACX_CHECK_XMVERSIONSTRING
dnl --------------
AC_DEFUN(ACX_CHECK_XMVERSIONSTRING,
[
  AC_CACHE_CHECK( "whether _XmVersionString[] can be referred to",
    acx_cv__xmversionstring,
    AC_TRY_LINK([#include <stdio.h>],
                [extern char _XmVersionString[[]]; printf("%s\n", _XmVersionString);],
                [acx_cv__xmversionstring="yes"],
                [acx_cv__xmversionstring="no"]
    )
  )
  if test "$acx_cv__xmversionstring" = "yes"
  then
    AC_DEFINE(HAVE__XMVERSIONSTRING)
    $1
  else
    :
    $2
  fi
])dnl

dnl ACX_PROG_GXX_PICKY_XHEADER
dnl -----------------------
dnl See if g++ complains about your X headers declaring functions with
dnl implicit return types.  This is known to occur with g++ 2.95 and later,
dnl compiling on Solaris.  The fix is to add the X include directory
dnl to the search path with "-isystem" instead of just "-I"; this makes
dnl g++ suppress the error messages.
dnl
AC_DEFUN(ACX_PROG_GXX_PICKY_XHEADER,
[
  AC_REQUIRE([AC_PROG_CXX])dnl
  AC_REQUIRE([AC_PATH_XTRA])dnl
  if test "$GXX" = yes; then
    AC_CACHE_CHECK([whether g++ gripes about your X headers],
      acx_prog_gxx_picky_xheader,
      [
	AC_LANG_SAVE
	AC_LANG_CPLUSPLUS
	ACX_SAVE_STATE
	CXXFLAGS="$CXXFLAGS $X_CFLAGS"
	AC_TRY_COMPILE([#include <X11/Intrinsic.h>], [int a;],
          acx_prog_gxx_picky_xheader=no,
	  [
	    CXXFLAGS="$CXXFLAGS -isystem $x_includes"
	    AC_TRY_COMPILE([#include <X11/Intrinsic.h>], [int a;],
	      acx_prog_gxx_picky_xheader=yes,
              AC_MSG_ERROR([Unable to compile with <X11/Intrinsic.h>])
	    )
	  ]
	)
	ACX_RESTORE_STATE
	AC_LANG_RESTORE
      ]
    )
    if test $acx_prog_gxx_picky_xheader = yes; then
      CXXFLAGS="$CXXFLAGS -isystem $x_includes"
    fi
  fi
])

dnl ACX_USE_DMALLOC
dnl -----------------------
dnl See if the dmalloc debugging-malloc library is available.
dnl See <http://dmalloc.com/> for the package.  The cached value is
dnl the base name of the library to use or "no" if tha package isn't
dnl able to be used.
dnl
AC_DEFUN(ACX_USE_DMALLOC,
[
  AC_REQUIRE([AC_PROG_CXX])dnl
  AC_SUBST(DMALLOC_LIB)
  AC_CACHE_CHECK([for working dmalloc], acx_use_dmalloc,
    [
      AC_CHECK_HEADER(dmalloc.h,
        [
	  AC_CHECK_LIB(dmallocxx, malloc,
	    acx_use_dmalloc=dmallocxx,
	    [
	       AC_CHECK_LIB(dmalloc, malloc,
	         acx_use_dmalloc=dmalloc,
	         acx_use_dmalloc=no
	      )
	    ]
	  )
        ],
	acx_use_dmalloc=no
      )
    ]
  )
  if test $acx_use_dmalloc != no; then
    AC_DEFINE(WITH_DMALLOC)
    DMALLOC_LIB="-l${acx_use_dmalloc}"
  fi
])

dnl ACX_CHECK_OPENSSL
dnl --------------
dnl Check for OpenSSL libraries
AC_DEFUN(ACX_CHECK_OPENSSL,
[
  AC_SUBST(OPENSSL_LIBS)
  AC_ARG_WITH(openssl_libraries,
  [  --with-openssl-libraries=OBJS    use OBJS as OPENSSL libraries [-lssl -lcrypto]],
  openssl_libraries="$withval")
  if test "x$openssl_libraries" = "x"
  then
    openssl_libraries="-lssl -lcrypto"
  fi

  AC_CACHE_CHECK( "for OpenSSL \>= $1", acx_cv_openssl,
    AC_CACHE_VAL(acx_cv_openssl_libraries, acx_cv_openssl_libraries=$openssl_libraries)
    ACX_SAVE_STATE
    LIBS="$acx_cv_openssl_libraries $LIBS"
    AC_TRY_RUN([
#include <stdlib.h>
#include <openssl/opensslv.h>
extern const char *SSL_version_str;
      int main(void) {
        char *vinc, *vlib;
        vinc = OPENSSL_VERSION_TEXT;
        vlib = (char *) SSL_version_str;
        if (strcmp(vinc, "OpenSSL [$1]") < 0) {
          exit(1);
        }
        if (strcmp(vinc, vlib) != 0) {
          exit(2);
        }
        exit(0);
      }
      ],

      acx_cv_openssl="yes",
      acx_cv_openssl="no",
      acx_cv_openssl="no"
    )
    ACX_RESTORE_STATE
  )
  if test "$acx_cv_openssl" = "yes"
  then
    OPENSSL_LIBS="$acx_cv_openssl_libraries"
    $2
  else
    OPENSSL_LIBS=
    $3
  fi
])dnl
