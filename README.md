# Host Logger

The main purpose of the Host Logger project is to handle and store host's
console output data, such as boot logs or Linux kernel messages printed to the
system console.

## Architecture
Host Logger is a standalone service (daemon) that works on top of the
obmc-console-server module and uses its UNIX domain socket to read the console
output.
```
+-------------+                                       +----------------+
|     Host    | State  +---------------------+ Event  |  Host Logger   |
|             +------->|        D-Bus        |------->|                |
|             |        +---------------------+        | +------------+ |
|             |                                  D +--->| Log buffer | |
|             |        +---------------------+   a |  | +------+-----+ |
|             |        | obmc-console-server |   t |  |        |       |
| +---------+ |  Data  |   +-------------+   |   a |  | +------+-----+ |
| | console |--------->|   | UNIX socket |---------+  | |  Log file  | |
| +---------+ |        |   +-------------+   |        | +------------+ |
+-------------+        +---------------------+        +----------------+
```
Unlike the obmc-console project, where buffer with console output is a
"black box" and exist only as a binary byte array, the Host Logger service
interprets the input stream: splits it into separated messages, adds a time
stamp and pushes the message into an internal buffer.
The buffer will be written to a file when one of the external events occurs,
such as a request for turn on the host or direct call of the function `Flush`
via D-Bus.
It gives us the ability to save the last boot log and subsequent messages in
separated files.
The service uses a set of rules (policy) that defines how large a buffer can
be (limit by time or message numbers), in which cases the buffer should be
flushed to a file, and how many previous log files can be saved on a flash
drive (log rotation).


## Configuration

The policy can be defined in two ways:
- By options passed to the "configure" script during the build process, this
  method sets up the default policy used by the service;
- By options passed to the executable module of the service, they can be
  specified as command line parameters inside the systemd service file
  used to launch the service.

### Limit of message buffer size
- `--szlimit`: Set maximum number of messages stored inside the buffer. Default
  value is `3000`. The value `0` will disable the rule.
- `--tmlimit`: Set maximum age of a message (in hours), all messages that older
  then the specified value will be cleaned. This rule can be used to store, for
  example, only messages for the latest hour. Default value is `0` (rule
  disabled).
Both of these options can be combined together, in this case a message will
be cleaned if it complies any of limit types. You can create an unlimited
buffer by passing `0` for both of these options.

### File write policy (buffer flush)
- `--path`:  Set the path used to store log files, default values is
  /var/lib/obmc/hostlogs.
- `--flush`: Set the period (in hours) to flush buffer to a file. Default value
  is `0`, which means that the flush operation will be started at every
  significant host state change - power on/off or OS boot complete events.

### File rotation policy
- `--rotate`: Set the maximum number of files stored on the flash memory.
  Default value is `10`. The service will remove the oldest files during buffer
  the flush procedure.


## Integration
Directory "integration" contains example files used to integrate the service
into OpenBMC system:
- `systemd.service`: systemd unit file used to describe the service;
- `dreport.plugin`: plugin for dreport module, used to add host's logs into
  a dump archive.

## D-Bus
The service provides an implementation of a D-Bus interface
`xyz.openbmc_project.HostLogger.service`.
Currently it has only one method `Flush` used for manually flushing logs.


## Example
```
# ls /var/lib/obmc/hostlogs
host_20181024_092655.log.gz  host_20181025_120520.log.gz
host_20181024_114342.log.gz  host_20181025_120910.log.gz

# zcat /var/lib/obmc/hostlogs/host_20181025_120910.log.gz
[ 12:05:30 ]: >>> Log collection started at 25.10.2018 12:05:30
[ 12:05:30 ]: --== Welcome to Hostboot v2.1-199-g38a9f30/hbicore.bin/ ==--
[ 12:05:34 ]:   5.52713|ISTEP  6. 3 - host_init_fsi
[ 12:05:34 ]:   6.13176|ISTEP  6. 4 - host_set_ipl_parms
[ 12:05:34 ]:   6.13309|ISTEP  6. 5 - host_discover_targets
...
```
