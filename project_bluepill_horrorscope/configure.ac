#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([STM_HAL], [0.1], [nils@we155.de])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET
AC_SUBST(AM_CXXFLAGS, [-std=c++14])
AC_SUBST(AM_CXXFLAGS, [-std=c++17])
AC_SUBST(AM_CXXFLAGS, [-pedantic])
AC_SUBST(AM_CXXFLAGS, [-Wall])

AC_PATH_PROG(CPPCHECKER, cppcheck, [not_found])
AC_PATH_PROG(GIT, git, [not_found])
AC_PATH_PROG(UNCRUSTIFY, uncrustify, [not_found])
AC_PATH_PROG(OPENSSL, openssl, [not_found])
AC_PATH_PROG(DOXYGEN, doxygen, [not_found])
AC_PATH_PROG(GENHTML, genhtml, [not_found])
AC_PATH_PROG(LCOV, lcov, [not_found])
AC_PATH_PROG(GCOV, gcov, [not_found])

if test -z "$OPENSSL" || test "X$OPENSSL" = "Xnot_found"; then 
	AC_MSG_ERROR([openssl not found!]) 
fi 

if test -z "$UNCRUSTIFY" || test "X$UNCRUSTIFY" = "Xnot_found"; then 
	AC_MSG_ERROR([uncrustify not found!]) 
fi 

if test "$($UNCRUSTIFY -v)" != "Uncrustify-0.67_f"; then 
	AC_MSG_ERROR([wrong uncrustify version. Get version 0.67_f: https://sourceforge.net/projects/uncrustify/files/uncrustify/uncrustify-0.67/uncrustify-0.67.tar.gz/download]) 
fi 



AC_DEFUN(AC_PATH_PROG_ARM_GCC, [
AC_PATH_PROG(ARM_GCC, arm-none-eabi-gcc, [not_found],[$PATH:/usr/local])
if test x"${ARM_GCC}" == x"not_found" ; then
    AC_MSG_RESULT([Searching for ARM_GCC with find!])
	ARM_GCC=$(find ~ -name 'arm-none-eabi-gcc' -print -quit)
else
	AC_MSG_RESULT([arm-none-eabi-gcc found!])
fi ])
AC_PATH_PROG_ARM_GCC

AC_DEFUN(AC_PATH_PROG_ARM_GPP, [
AC_PATH_PROG(ARM_GPP, arm-none-eabi-g++, [not_found],[$PATH:/usr/local])
if test x"${ARM_GPP}" == x"not_found" ; then
    AC_MSG_RESULT([Searching for ARM_GPP with find!])
	ARM_GPP=$(find ~ -name 'arm-none-eabi-g++' -print -quit)
else
	AC_MSG_RESULT([arm-none-eabi-g++ found!])
fi ])
AC_PATH_PROG_ARM_GPP

AC_DEFUN(AC_PATH_PROG_ARM_AR, [
AC_PATH_PROG(ARM_AR, arm-none-eabi-ar, [not_found],[$PATH:/usr/local])
if test x"${ARM_AR}" == x"not_found" ; then
    AC_MSG_RESULT([Searching for ARM_AR with find!])
	ARM_AR=$(find ~ -name 'arm-none-eabi-ar' -print -quit)
else
	AC_MSG_RESULT([arm-none-eabi-ar found!])
fi ])
AC_PATH_PROG_ARM_AR

AC_DEFUN(AC_PATH_PROG_ARM_LD, [
AC_PATH_PROG(ARM_LD, arm-none-eabi-ld, [not_found],[$PATH:/usr/local])
if test x"${ARM_LD}" == x"not_found" ; then
    AC_MSG_RESULT([Searching for ARM_LD with find!])
	ARM_LD=$(find ~ -name 'arm-none-eabi-ld' -print -quit)
else
	AC_MSG_RESULT([arm-none-eabi-ld found!])
fi ])
AC_PATH_PROG_ARM_LD

AC_DEFUN(AC_PATH_PROG_ARM_OBJCOPY, [
AC_PATH_PROG(ARM_OBJCOPY, arm-none-eabi-objcopy, [not_found],[$PATH:/usr/local])
if test x"${ARM_OBJCOPY}" == x"not_found" ; then
    AC_MSG_RESULT([Searching for ARM_OBJCOPY with find!])
	ARM_OBJCOPY=$(find ~ -name 'arm-none-eabi-objcopy' -print -quit)
else
	AC_MSG_RESULT([arm-none-eabi-objcopy found!])
fi ])
AC_PATH_PROG_ARM_OBJCOPY

AC_DEFUN(AC_PATH_PROG_ARM_SIZE, [
AC_PATH_PROG(ARM_SIZE, arm-none-eabi-size, [not_found],[$PATH:/usr/local])
if test x"${ARM_OBJCOPY}" == x"not_found" ; then
    AC_MSG_RESULT([Searching for ARM_SIZE with find!])
	ARM_OBJCOPY=$(find ~ -name 'arm-none-eabi-size' -print -quit)
else
	AC_MSG_RESULT([arm-none-eabi-size found!])
fi ])
AC_PATH_PROG_ARM_SIZE

# Checks for libraries.
AC_CHECK_LIB([pthread], pthread_create)
AC_CHECK_LIB([thread], sleep_for)
AC_CHECK_LIB([crypto], SHA256)

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([clock_gettime floor getcwd gettimeofday inet_ntoa localtime_r memset pow select socket sqrt strstr strtoul])

PROJECT_PATH="$(pwd)"
AC_SUBST(PROJECT_PATH)


AC_CONFIG_FILES([Makefile ../utilities/pre-commit.settings])
AC_OUTPUT


rm ../.git/hooks/pre-commit
ln -Fv ../utilities/pre-commit.uncrustify ../.git/hooks/pre-commit
chmod a+x ../.git/hooks/pre-commit
echo Added pre-commit hook to this repository

rm ../.git/hooks/pre-push
ln -Fv ../utilities/pre-push ../.git/hooks/pre-push
chmod a+x ../.git/hooks/pre-push
echo Added pre-push hook to this repository

