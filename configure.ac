AC_PREREQ([2.69])
AC_INIT([phosphor-hostlogger],
        [m4_esyscmd_s([git describe --always --dirty --long])],
        [https://github.com/YADRO-KNS/phosphor-hostlogger], ,
        [https://github.com/YADRO-KNS/phosphor-hostlogger])
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
AX_APPEND_COMPILE_FLAGS([-fpic -Wall], [CFLAGS])
AX_APPEND_COMPILE_FLAGS([-fpic -Wall], [CXXFLAGS])

# Checks for external dependencies
AC_CHECK_LIB([z], [gzopen], [], [AC_MSG_ERROR(zlib not found)])
PKG_CHECK_MODULES([SYSTEMD], [libsystemd >= 221], ,
    AC_MSG_ERROR(["systemd required but not found."]))
PKG_CHECK_MODULES([PHOSPHOR_DBUS_INTERFACES], [phosphor-dbus-interfaces], ,
    AC_MSG_ERROR(["Requires phosphor-dbus-interfaces package."]))
PKG_CHECK_MODULES([SDBUSPLUS], [sdbusplus], ,
    AC_MSG_ERROR(["Requires sdbusplus package."]))
AC_PATH_PROG([SDBUSPLUSPLUS], [sdbus++])
AS_IF([test "x$SDBUSPLUSPLUS" == "x"],
    AC_MSG_ERROR(["Requires sdbus++"]))

# Configuration property: path to the ouput directory of log files
AC_ARG_VAR([LOG_OUTPUT_PATH], [Path used to store logs.])
AS_IF([test "x$LOG_OUTPUT_PATH" == "x"], [LOG_OUTPUT_PATH="/var/lib/obmc/hostlogs"])
AC_DEFINE_UNQUOTED([LOG_OUTPUT_PATH], ["$LOG_OUTPUT_PATH"])

# Configuration property: log storage limit (maximum number of messages to store)
AC_ARG_VAR([LOG_STORAGE_SIZE_LIMIT], [Limit number of messages in store, default is 3000])
AS_IF([test "x$LOG_STORAGE_SIZE_LIMIT" == "x"], [LOG_STORAGE_SIZE_LIMIT=3000])
AC_DEFINE_UNQUOTED([LOG_STORAGE_SIZE_LIMIT], [${LOG_STORAGE_SIZE_LIMIT}])

# Configuration property: log storage limit (oldest time in hours)
AC_ARG_VAR([LOG_STORAGE_TIME_LIMIT], [Limit message store by oldest time in hours, default is 0])
AS_IF([test "x$LOG_STORAGE_TIME_LIMIT" == "x"], [LOG_STORAGE_TIME_LIMIT=0])
AC_DEFINE_UNQUOTED([LOG_STORAGE_TIME_LIMIT], [${LOG_STORAGE_TIME_LIMIT}])

# Configuration property: flush period policy.
AC_ARG_VAR([LOG_FLUSH_PERIOD], [Set the default flush period time, default is 0])
AS_IF([test "x$LOG_FLUSH_PERIOD" == "x"], [LOG_FLUSH_PERIOD=0])
AC_DEFINE_UNQUOTED([LOG_FLUSH_PERIOD], [${LOG_FLUSH_PERIOD}])

# Configuration property: limit for log rotation - maximum number of files to store in the log's directory
AC_ARG_VAR([LOG_ROTATION_LIMIT], [Limit for log rotation, default is 10])
AS_IF([test "x$LOG_ROTATION_LIMIT" == "x"], [LOG_ROTATION_LIMIT=10])
AC_DEFINE_UNQUOTED([LOG_ROTATION_LIMIT], [${LOG_ROTATION_LIMIT}])

# Debug property
AC_ARG_ENABLE(debug,
    [AS_HELP_STRING([--enable-debug], [enable debugging [disabled]])],
    [AM_CPPFLAGS="$AM_CPPFLAGS -DDEBUG -g -O0"]
    [AM_CPPFLAGS="$AM_CPPFLAGS -DNDEBUG"]
)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
