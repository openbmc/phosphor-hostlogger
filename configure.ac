AC_PREREQ([2.69])
AC_INIT([phosphor-hostlogger],
        [m4_esyscmd_s([git describe --always --dirty --long])],
        [https://github.com/openbmc/phosphor-hostlogger/issues])
AC_LANG([C++])
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror foreign])
AM_SILENT_RULES([yes])

# Check for programs
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AC_HEADER_STDC

# Compiler flags
AX_CXX_COMPILE_STDCXX([17], [noext])
AX_APPEND_COMPILE_FLAGS([-fpic -Wall], [CXXFLAGS])

# Checks for external dependencies
AC_CHECK_LIB([z], [gzopen], [],
             [AC_MSG_ERROR([zlib required but not found])])
PKG_CHECK_MODULES([SYSTEMD], [libsystemd >= 221], [],
                  [AC_MSG_ERROR([systemd required but not found])])
PKG_CHECK_MODULES([PHOSPHOR_DBUS_INTERFACES], [phosphor-dbus-interfaces], [],
                  [AC_MSG_ERROR([phosphor-dbus-interfaces required but not found])])
PKG_CHECK_MODULES([SDBUSPLUS], [sdbusplus], [],
                  [AC_MSG_ERROR([sdbusplus required but not found])])
AC_PATH_PROG([SDBUSPLUSPLUS], [sdbus++])
AS_IF([test "x$SDBUSPLUSPLUS" = "x"],
      [AC_MSG_ERROR([sdbus++ required but not found])])

# systemd unit general configuration
PKG_PROG_PKG_CONFIG
AC_ARG_WITH([systemdsystemunitdir],
            [AS_HELP_STRING([--with-systemdsystemunitdir=DIR],
                            [Directory for systemd service files])], [],
            [with_systemdsystemunitdir=auto])
AS_IF([test "x$with_systemdsystemunitdir" = "xyes" -o "x$with_systemdsystemunitdir" = "xauto"],
      [def_systemdsystemunitdir=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)
       AS_IF([test "x$def_systemdsystemunitdir" = "x"],
             [AS_IF([test "x$with_systemdsystemunitdir" = "xyes"],
                    [AC_MSG_ERROR([systemd support requested but pkg-config unable to query systemd package])],
                    [with_systemdsystemunitdir=no])],
             [with_systemdsystemunitdir="$def_systemdsystemunitdir"])])
AS_IF([test "x$with_systemdsystemunitdir" != "xno"],
      [AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])])
AM_CONDITIONAL([HAVE_SYSTEMD], [test "x$with_systemdsystemunitdir" != "xno"])

# systemd unit parameters
AC_ARG_VAR([SYSTEMD_TARGET], [systemd target for starting service, default is multi-user.target])
AS_IF([test "x$SYSTEMD_TARGET" = "x"], [SYSTEMD_TARGET=multi-user.target])
AC_ARG_VAR([HOST_TTY], [host teletype (TTY), default is ttyVUART0])
AS_IF([test "x$HOST_TTY" = "x"], [HOST_TTY=ttyVUART0])

# Configuration property: path to the ouput directory of log files
AC_ARG_VAR([LOG_OUTPUT_PATH], [Path used to store logs, default is /var/lib/obmc/hostlogs])
AS_IF([test "x$LOG_OUTPUT_PATH" = "x"], [LOG_OUTPUT_PATH="/var/lib/obmc/hostlogs"])
AC_DEFINE_UNQUOTED([LOG_OUTPUT_PATH], ["${LOG_OUTPUT_PATH}"])

# Configuration property: log storage limit (maximum number of messages to store)
AC_ARG_VAR([LOG_STORAGE_SIZE_LIMIT], [Limit number of messages in store, default is 3000])
AS_IF([test "x$LOG_STORAGE_SIZE_LIMIT" = "x"], [LOG_STORAGE_SIZE_LIMIT=3000])
AC_DEFINE_UNQUOTED([LOG_STORAGE_SIZE_LIMIT], [${LOG_STORAGE_SIZE_LIMIT}])

# Configuration property: log storage limit (oldest time in hours)
AC_ARG_VAR([LOG_STORAGE_TIME_LIMIT], [Limit message store by oldest time in hours, default is 0])
AS_IF([test "x$LOG_STORAGE_TIME_LIMIT" = "x"], [LOG_STORAGE_TIME_LIMIT=0])
AC_DEFINE_UNQUOTED([LOG_STORAGE_TIME_LIMIT], [${LOG_STORAGE_TIME_LIMIT}])

# Configuration property: flush period policy.
AC_ARG_VAR([LOG_FLUSH_PERIOD], [Set the default flush period time, default is 0])
AS_IF([test "x$LOG_FLUSH_PERIOD" = "x"], [LOG_FLUSH_PERIOD=0])
AC_DEFINE_UNQUOTED([LOG_FLUSH_PERIOD], [${LOG_FLUSH_PERIOD}])

# Configuration property: limit for log rotation - maximum number of files to store in the log's directory
AC_ARG_VAR([LOG_ROTATION_LIMIT], [Limit for log rotation, default is 10])
AS_IF([test "x$LOG_ROTATION_LIMIT" = "x"], [LOG_ROTATION_LIMIT=10])
AC_DEFINE_UNQUOTED([LOG_ROTATION_LIMIT], [${LOG_ROTATION_LIMIT}])

# dreport integration
AC_ARG_ENABLE([dreport],
              [AS_HELP_STRING([--enable-dreport], [enable dreport integration as plugin [yes]])],
              [case "${enableval}" in
                 yes) dreport=true ;;
                 no)  dreport=false ;;
                 *)   AC_MSG_ERROR([bad value ${enableval} for --enable-dreport]) ;;
               esac],
              [dreport=true])
AM_CONDITIONAL(HAVE_DREPORT, test "x$dreport" = "xtrue")

# Debug build configuration
AC_ARG_ENABLE([debug],
              [AS_HELP_STRING([--enable-debug], [turn on debugging [no]])],
              [case "${enableval}" in
                 yes) debug=true ;;
                 no)  debug=false ;;
                 *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
               esac],
              [debug=false])
AM_CONDITIONAL(DEBUG, test "x$debug" = "xtrue")

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([hostlogger.service])
AC_CONFIG_FILES([hostlogger.dreport])
AC_OUTPUT
