YAML_IFACE = xyz/openbmc_project/HostLogger.interface.yaml
DBUS_IFACE = xyz.openbmc_project.HostLogger
GENERATED_HPP = xyz/openbmc_project/HostLogger/server.hpp
GENERATED_CPP = xyz/openbmc_project/HostLogger/server.cpp

bin_PROGRAMS = hostlogger

nobase_nodist_include_HEADERS = \
	$(GENERATED_HPP)

hostlogger_SOURCES = \
	$(GENERATED_CPP) \
	src/main.cpp \
	src/dbus_server.hpp \
	src/dbus_server.cpp \
	src/dbus_watch.hpp \
	src/dbus_watch.cpp \
	src/log_manager.hpp \
	src/log_manager.cpp \
	src/log_storage.hpp \
	src/log_storage.cpp

hostlogger_CXXFLAGS = $(SDBUSPLUS_CFLAGS) $(PHOSPHOR_DBUS_INTERFACES_CFLAGS)
hostlogger_LDADD = $(SDBUSPLUS_LIBS) $(PHOSPHOR_DBUS_INTERFACES_LIBS)

if DEBUG
hostlogger_CXXFLAGS += -DDEBUG -g3 -O0
else
hostlogger_CXXFLAGS += -DNDEBUG
endif  # DEBUG

BUILT_SOURCES = $(GENERATED_HPP) $(GENERATED_CPP)
CLEANFILES = $(BUILT_SOURCES)

if HAVE_SYSTEMD
SYSTEMD_UNIT = hostlogger.service
systemdsystemunit_DATA = $(SYSTEMD_UNIT)
CLEANFILES += $(SYSTEMD_UNIT)
endif  # HAVE_SYSTEMD

if HAVE_DREPORT
CLEANFILES += hostlogger.dreport
endif  # HAVE_DREPORT

$(GENERATED_HPP): $(YAML_IFACE)
	@mkdir -p $(@D)
	$(SDBUSPLUSPLUS) -r $(srcdir) interface server-header $(DBUS_IFACE) > $@

$(GENERATED_CPP): $(YAML_IFACE) $(GENERATED_HPP)
	@mkdir -p $(@D)
	$(SDBUSPLUSPLUS) -r $(srcdir) interface server-cpp $(DBUS_IFACE) > $@
