# Host Logger

The main purpose of the Host Logger project is to handle and store host's
console output data, such as boot logs or Linux kernel messages printed to the
system console.

There are two modes in Host Logger.

Buffer mode: host logs are stored in a temporary buffer and flushed to a file
according to the policy that can be defined with service parameters. It gives
the ability to save the startup logs and shutdown logs in separate files.

Stream mode: host logs are forwarded into a server socket (e.g. a socket created
by the rsyslog imuxsock module). It gives the ability to stream logs in almost
realtime.

## Architecture

Host Logger is a standalone service (daemon) that works on top of the
obmc-console and uses its UNIX domain socket to read the console output.

### The Buffer Mode
```
+-------------+                                       +----------------+
|    Host     | State  +---------------------+ Event  |   Host Logger  |
|             |------->|        D-Bus        |------->|                |
|             |        +---------------------+        | +------------+ |
|             |                                  D +--->| Log buffer | |
|             |        +---------------------+   a |  | +------+-----+ |
|             |        | obmc-console-server |   t |  |        V       |
| +---------+ |  Data  |   +-------------+   |   a |  | +------------+ |
| | console |--------->|   | UNIX socket |---------+  | |  Log file  | |
| +---------+ |        |   +-------------+   |        | +------------+ |
+-------------+        +---------------------+        +----------------+
```

Unlike the obmc-console project, where console output is a binary byte stream,
the service in buffer mode interprets this stream: splits it into separate
messages, adds a time stamp and pushes the message into an internal buffer.
Maximum size of the buffer and flush conditions are controlled by service
parameters.

### The Stream Mode

```
+-----------+                               +-------------+      +------------+
|           |                               |             |      |            |
|   Host    |                               | Host Logger |      |  rsyslog   |
|           |                               |             |      | +--------+ |  +---------+
|           |                               |             |      | | omfile ----->Log Files|--+
|           |   +--------------------+      |             |      | +--------+ |  +---------+  |
|           |   |obmc-console-server |      |             |      |            |  +------------v---+
|+---------+|   |   +-------------+  |Stream|+-----------+|DGRAM | +--------+ |  | Redfish        |
|| console -----|-->| UNIX socket |---------->  Service  --------->|imuxsock| |  | LogService &   |
|+---------+|   |   +-------------+  |      |+-----------+|      | +--------+ |  | EventService   |
+-----------+   +--------------------+      +-------------+      +------------+  +----------------+
```

The service in stream mode forwards the byte stream into rsyslog via the imuxsock
module. The log is persisted via the omfile module as soon as collected. It makes
Host Logger leverage exsisting tools (rsyslog and logrotate). It also fits in the
Redfish LogService and EventService architecture in OpenBMC.

## Log buffer rotation policy

### The Buffer Mode

Maximum buffer size can be defined in the service configuration using two ways:
- Limits by size: buffer will store the last N messages, the oldest messages are
  removed. Controlled by `BUF_MAXSIZE` option.
- Limits by time: buffer will store messages for the last N minutes, oldest
  messages are removed. Controlled by `BUF_MAXTIME` option.

Any of these parameters can be combined.

### The Stream Mode

Rotation and compression are handled by the [logrotate](https://linux.die.net/man/8/logrotate)
tool.

## Log buffer flush policy

### The Buffer Mode

Messages from the buffer will be written to a file when one of the following
events occurs:
- Host changes its state (start, reboot or shut down). The service watches the
  state via the D-Bus object specified in `HOST_STATE` parameter.
- Size of the buffer reaches its limits controlled by `BUF_MAXSIZE` and
  `BUF_MAXTIME` parameters, this mode can be activated by `FLUSH_FULL` flag.
- Signal `SIGUSR1` is received (manual flush).

### The Stream Mode

Logs are flushed as soon as they are collected.

## Configuration

Configuration of the service is loaded from environment variables, so each
instance of the service can have its own set of parameters.
Environment files are stored in the directory `/etc/hostlogger` and must have
the extension `conf`. The file name is the name of the associated Host logger
instance and the instance of the obmc-console service (e.g. `ttyVUART0`).

### Environment variables

Any of these variables can be omitted, in which cases default values are used.
If variable's value has an invalid format, the service fails with an error.

- `SOCKET_ID`: Socket Id used for connection with the host console. This Id
  shall match the "socket-id" parameter of obmc-console server.
  The default value is empty (single-host mode).
- `MODE`: The mode that the service is running in. Possible values: `buffer`
  or `stream`. The default value is `buffer`.

#### The Buffer Mode

- `BUF_MAXSIZE`: Max number of stored messages in the buffer. The default value
  is `3000` (0=unlimited).

- `BUF_MAXTIME`: Max age of stored messages in minutes. The default value is
  `0` (unlimited).

- `FLUSH_FULL`: Flush collected messages from buffer to a file when one of the
  buffer limits reaches a threshold value. At least one of `BUF_MAXSIZE` or
  `BUF_MAXTIME` must be defined. Possible values: `true` or `false`. The default
  value is `false`.

- `HOST_STATE`: Flush collected messages from buffer to a file when the host
  changes its state. This variable must contain a valid path to the D-Bus object
  that provides host's state information. Object shall implement interfaces
  `xyz.openbmc_project.State.Host` and `xyz.openbmc_project.State.OperatingSystem.Status`.
  The default value is `/xyz/openbmc_project/state/host0`.

- `OUT_DIR`: Absolute path to the output directory for log files. The default
  value is `/var/lib/obmc/hostlogs`.

- `MAX_FILES`: Log files rotation, max number of files in the output directory,
  oldest files are removed. The default value is `10` (0=unlimited).

#### The Stream Mode

- `STREAM_DST`: Absolute path to the output unix socket. The default value is
  `/run/rsyslog/console_input`.

### Example

#### Remove file limits

Relevant for OpenPOWER based servers, using `ttyVUART0` for console.

File `phosphor-hostlogger_%.bbappend`:
```
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI_append = " file://ttyVUART0.conf"
```

File `phosphor-hostlogger/ttyVUART0.conf`:
```
MAX_FILES=0
```

#### Custom object for host state monitoring

Relevant for x86 based servers: using `ttyS2` for console and
`/xyz/openbmc_project/state/os` for host state monitoring.

File `phosphor-hostlogger_%.bbappend`:
```
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
SRC_URI_append = " file://ttyS2.conf"
```

File `phosphor-hostlogger/ttyS2.conf`:
```
HOST_STATE=/xyz/openbmc_project/state/os
```

## Multi-host support

The single instance of the service can handle only one host console at a time.
If OpenBMC has multiple hosts, the console of each host must be associated with
its own instance of the Host Logger service. This can be achieved using the
systemd unit template.
