# Host Logger

The main purpose of the Host Logger project is to handle and store host's
console output data, such as boot logs or Linux kernel messages printed to the
system console.

Host logs are stored in a temporary buffer and flushed to a file according to
the policy that can be defined with service parameters. It gives the ability to
save the last boot log and subsequent messages in separated files.

## Architecture

Host Logger is a standalone service (daemon) that works on top of the
obmc-console and uses its UNIX domain socket to read the console output.

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
the Host Logger service interprets this stream: splits it into separated
messages, adds a time stamp and pushes the message into an internal buffer.
Maximal size of the buffer and flush conditions are controlled by service
parameters.

## Log buffer rotation policy

Maximal buffer size can be defined with service configuration in two ways:
- Limits by size: buffer will store last N messages, oldest messages are
  removed. Controlled by `BUF_MAXSIZE` option.
- Limits by time: buffer will store messages for last N minutes, oldest messages
  are removed. Controlled by `BUF_MAXTIME` option.

Any of these parameters can be combined.

## Log buffer flush policy

Messages from the buffer will be written to a file when one of the following
events occurs:
- Host changes its state (start, reboot or shut down). The service watches the
  state via D-Bus object specified in `HOST_STATE` parameter.
- Size of the buffer reaches its limits controlled by `BUF_MAXSIZE` and
  `BUF_MAXTIME` parameters.
- Signal `SIGUSR1` is received (manual flush).

## Configuration

Configuration of the service is loaded from environment variables, so each
instance of the service can have its own set of parameters.
Environment files are stored in the directory `/etc/hostlogger`. File name
must be in format `{INSTANCE}.conf`.

### Environment variables

Any of these variables can be absent, in such cases default values are used.
If variable's value has invalid format, service fails with error.

- `SOCKET_ID`: Socket Id used for connection with the host console. This Id
  shall conform to the "socket-id" parameter of obmc-console server.
  Default value is empty (single-host mode).

- `BUF_MAXSIZE`: Max number of stored messages in the buffer. Default value
  is `3000` (0=unlimited).

- `BUF_MAXTIME`: Max age of stored messages in minutes. Default value is
  `0` (unlimited).

- `FLUSH_FULL`: Flush collected messages from buffer to a file when one of the
  buffer limits reaches a threshold value. At least one of `BUF_MAXSIZE` or
  `BUF_MAXTIME` must be defined. Possible values: `true` or `false`. Default
  value is `false`.

- `HOST_STATE`: Flush collected messages from buffer to a file when the host
  changes its state. This variable must contain a valid path to the D-Bus object
  that provides host's state information. Object shall implement interfaces
  `xyz.openbmc_project.State.Host` and `xyz.openbmc_project.State.OperatingSystem.Status`.
  Default value is `/xyz/openbmc_project/state/host0`.

- `OUT_DIR`: Absolute path to the output directory for log files. Default value
  is `/var/lib/obmc/hostlogs`.

- `MAX_FILES`: Log files rotation, max number of files in the output directory,
  oldest files are removed. Default value is `10` (0=unlimited).

## Multi-host support

The single instance of the service can handle only one host console at a time.
If OpenBMC has multiple hosts, the console of each host must be associated with
its own instance of the Host Logger service. This can be achieved using the
systemd unit template.
