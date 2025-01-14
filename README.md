# McStas Readout Master for ESS detectors
The Readout [McStas 3.3+](https://mcstas.org) component is the link between ray-tracing neutron simulations
and the ESS data pipeline via the [Event Formation Unit](https://github.com/ess-dmsc/event-formation-unit) (EFU).

## Overview
The component makes a Poisson distributed Monte Carlo choice for every neutron
that it is passed based on the weight, `_particle->p`, or the weight squared (user configurable).
If the random choice is finite event-identifying parameters are read from the `_particle` struct's `USER_VARS`
(detailed below) which must have been set earlier in the simulation.
If the random choice is zero the component can, optionally, produce random noise values for all require parameters
or alternatively discard the neutron.
If not discarded, the information is added to a network packet through the C interface `Readout`
and ultimately sent to the EFU by the C++ object `ReadoutClass`.

The exact details of information required by the Event Formation Unit depend on the type of detector and
are explained in a number of 'Detector Interface Control Documents', 
produced by the Experiment Control and Data Curation division of the Data Management & Software Centre.

## Common McStas component parameters
| Parameter      | Type   | Description                                                                              |
|----------------|--------|------------------------------------------------------------------------------------------|
| `ring`         | named  | identifies the Readout Ring                                                              |
| `fen`          | named  | identifies the Front End Node                                                            |
| `tof`          | named  | time-of-flight of the neutron, default: `_particle->t`, any `USER_VARS` value is valid   |
| `ip`           | string | the resolvable domain name or IP address of the EFU which will receive the packets       |
| `port`         | int    | the EFU event-packet UDP port, 9000 by default to match the EFU default                  |
| `command_port` | int    | the EFU command TCP port, 10800 by default to match the EFU default                      |
| `broadcast`    | int    | flag to control if event packets are sent, on by default                                 |
| `pulse_rate`   | double | the reference time pulse rate, used in calculating EFU-required time stamps              |
| `noisy`        | int    | flag to control if otherwise-dropped events should be replaced by noise, off by default  |
| `noise_level`  | double | if `noisy`, *how* noisy &mdash; fractional probability of noise: 0.1 by default          |
| `event_mode`   | string | `"p"` for `rand_poisson(_particle->p)`, `"pp"` for p<sup>2</sup>                         |
| `verbose`      | int    | controls `STDOUT` printing: -1=silent, 0=errors, 1=warnings, 2=info, 3=details           |
| `ess_type`     | int    | identifies simulated ESS readout &mdash; BIFROST: 0x34 (dec 52), CSPEC: 0x40 (dec 64)    |
| `filename`     | string | if present, neutron ray data provided to the broadcaster will be stored to HDF5 filename | 


## Common Event Formation Unit parameters
The event formation unit requires a number of parameters to identify the detector element and time of flight which
comprise the events it forwards to Kafka.
These parameters must be stored in the `USER_VARS` contained in the `_particle` `struct` which *should* be set
appropriately earlier in the McStas instrument.
The component identifies the `struct` parameters *by name* and requires the names as input component parameters.
The component parameter names common to all readout components are the Ring identifier, Front End Node identifier,
and time of flight.
Finally, the EFU requires the event time, the most-recent reference time, and the previous reference time to determine
the *relative* event time which is ultimately stored in a NeXus file.
These times must be encoded as two integers: the `high` time, which is the number of whole seconds since the UNIX epoch,
`1970-01-01T00:00Z`, and the `low` time, which is the number of `88052499 Hz` clock ticks since the last whole second.

The reference times are determined automatically based on the component parameter `pulse_rate` and the running time of
the McStas simulation.
The event time is set to the most-recent reference time plus neutron time-of-flight, which is read from `_particle->t`
by default but can be overridden by setting the component parameter `tof`.

| Named parameter | EFU parameter         |
|-----------------|-----------------------|
| `ring`          | `RingId`              |
| `fen`           | `FENId`               |
| `tof`           | `HighTime`, `LowTime` |


# `Readout.comp`: Subprocess EFU, CAEN electronics [deprecated]
Using `EFU` produced `HDF5` files along with `mcrun` type scans requires that the `EFU` be stopped after every scan point.
This component can control the execution of a local `EFU` by starting it in a subprocess and instructing it to stop
via its command-port interface.

At the moment, it only supports CAEN instruments and should probably not be used going forward.

## McStas Component specific parameters
| Parameter        | Type   | Description                                                                             |
|------------------|--------|-----------------------------------------------------------------------------------------|
| `tube`           | named  | identifies the pair of digitizers connected to the channel                              |
| `a`              | named  | is the integrated voltage output of the first digitizer                                 |
| `b`              | named  | is the integrated voltage output of the second digitizer                                |
| `receiver`       | string | the path to the EFU executable, required if it is to be started locally                 |
| `options`        | string | command-line options used when starting the EFU                                         |
| `filename`       | string | filename for events if EFU started and output requested, `NAME_CURRENT_COMP` by default |
| `hdf5_output`    | int    | flag to control dumping EFU-received events if EFU started, on by default               |
| `start_receiver` | int    | flag to control whether the local EFU should be started, on by default                  |

To identify in which channel an event occurred, the CAEN variant of the EFU requires an additional integer`TubeId`.
This identifies the pair of physical channels in the CAEN digitizer which produced an event.
The position between the two end of the detecting tube is reconstructed using the integrated amplitudes output
as part of the event message by the CAEN digitizer, which are therefore required as input to the CAEN EFU
as `AmpA` and `AmpB`.

| Named parameter | EFU parameter |
|-----------------|---------------|
| `tube`          | `TubeId`      |
| `a`             | `AmpA`        |
| `b`             | `AmpB`        |


# `ReadoutCAEN.comp`: <sup>3</sup>He instruments with CAEN electronics

A version of `Readout.comp` extended to support four integrated amplitudes per event, 
which is not able to start or stop the EFU.
This requires that the EFU be run independent of the McStas process.
This component is intended to be used with the full ECDC readout stack,
which makes use of a Kafka broker and independent file writers. 

## Component specific parameters
| Parameter   | Type  | Description                                                |
|-------------|-------|------------------------------------------------------------|
| `tube`      | named | identifies the pair of digitizers connected to the channel |
| `a`         | named | is the integrated voltage output of the first digitizer    |
| `b`         | named | is the integrated voltage output of the second digitizer   |
| `c`         | named | is the integrated voltage output of the third digitizer    |
| `d`         | named | is the integrated voltage output of the fourth digitizer   |
| `fen_value` | int   | used for `FENId` if no named `fen` parameter               |
| `c_value`   | int   | used when `c` is not named; default 0                      |
| `d_value`   | int   | used when `d` is not named; default 0                      |


# `ReadoutTTLMonitor.comp`: simple TTL based beam monitors

Like `ReadoutCAEN.comp` this component can not control the EFU it will communicate with.
It is intended to work with _one_ simulated TTL beam monitor, since it can only handle a single time of flight.

An instrument with multiple beam monitors would require multiple `ReadoutTTLMonitor.comp` instances
in order to output all of their events.
To support this operation for 0-D monitors, it is possible to define a static `Ring`, `FEN`, `Pos` and `Channel`,
and then place this component *directly* after each `EXTEND`ed monitor which defines an appropriate `ADC` value.

```instr
...
USER_VARS %{
  ...
  int monitor_signal;
%}
TRACE
...
monitor0 = Monitor(xwidth=x, yheight=y, restore_neutron=1) AT (...) EXTEND %{
  monitor_signal = (SCATTER) ? 500 + (int)(rand01() * 1024) : 0;
%}
output0 = ReadoutTTLMonitor(channel_value=0, fen_value=100, adc="monitor_signal", ip="ttl_monitor_efu", port=9001);
...
monitor1 = Monitor(xwidth=x, yheight=y, restore_neutron=1) AT (...) EXTEND %{
  monitor_signal = (SCATTER) ? 500 + (int)(rand01() * 1024) : 0;
%}
output1 = ReadoutTTLMonitor(channel_value=1, fen_value=100, adc="monitor_signal", ip="ttl_monitor_efu", port=9001);
...
monitor2 = Monitor(xwidth=x, yheight=y, restore_neutron=1) AT (...) EXTEND %{
  monitor_signal = (SCATTER) ? 500 + (int)(rand01() * 1024) : 0;
%}
output2 = ReadoutTTLMonitor(channel_value=2, fen_value=100, adc="monitor_signal", ip="ttl_monitor_efu", port=9001);
...
```

## Additional component parameters
| Parameter       | Type  | Description                                        |
|-----------------|-------|----------------------------------------------------|
| `pos`           | named | detection position in monitor                      |
| `channel`       | named | identifies the monitor                             |
| `adc`           | named | the integrated voltage output of the digitizer     |
| `ring_value`    | int   | used for `RingId` if no named `ring` parameter     |
| `fen_value`     | int   | used for `FENId` if no named `fen` parameter       |
| `pos_value`     | int   | used for `Pos` if no named `pos` parameter         |
| `channel_value` | int   | used for `Channel` if no named `channel` parameter |


# Installation

After cloning this repository, use CMake to configure, build, and install the shared C library.

To install the library and configuration-query tool within the current user's home directory, e.g., execute 
```bash
git clone https://github.com/g5t/mcstas-readout-master.git
cmake -S mcstas-readout-master -B mcstas-readout-master-build -DCMAKE_INSTALL_PREFIX=~/.local
cmake --build mcstas-readout-master-build --target install
```

## Automatic EFU start/stop and HDF5 raw event file [deprecated]
Performing scans in McStas which produce raw EFU-captured event files will be necessary to simulate some calibration procedures.
The `Readout.comp` McStas component can spawn a second process and start the EFU (or other packet receiver) before the simulation of one scan point,
and stop it at the end of the scan point.
By default the output HDF5 files are placed within the McStas output directory, and can therefore be associated with the scan points directly.

## MPI Support
McStas can be run on any number of MPI workers. If any of the `Readout` components are run in MPI all nodes should have network
access to the host running the EFU(s). If the EFU is controlled by the `Readout` component, it will be started only by the
master MPI node.

# Use
Once installed as above, you can include the readout component in an exising McStas instrument by placing something
similar to the following in the TRACE section of an instrument file (likely at the end)
```instr
TRACE
  ...
  SEARCH SHELL "readout-config --show compdir"
  COMPONENT readout = Readout(ring="RING", fen="FEN", channel="TUBE", a="left", b="right", ...)
  AT (0, 0, 0) ABSOLUTE
  ...
```

If multiple Readout components are included in an instrument, the `SEARCH SHELL` command is only needed once.
Consult the McStas 3.3+ documentation for details of its use.

## Use with McStas 3.2
The `SEARCH SHELL` automatic path modifications used above to find the included components do not work prior to the
version 3.3 release of McStas.
It is possible to use the components with McStas version 3.2 if you copy the component files to one of the McStas
component search directories and then ommit the `SEARCH SHELL` line(s) in the instrument file.