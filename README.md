# McStas Readout Master for ESS <sup>3</sup>He tube instruments with CAEN electronics

The Readout [McStas 3.2+](https://mcstas.org) component is the link between ray-tracing neutron simulations
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

The exact details of information required by the Event Formation Unit are in a number of 
'Detector Interface Control Documents', produced by the Experiment Control and Data Curation division 
of the Data Management & Software Centre.

## Parameters of the McStas component
| Parameter          | Description                                                                                                 |
|--------------------|-------------------------------------------------------------------------------------------------------------|
| `ring`             | identifies the Readout Ring                                                                                 |
| `fen`              | identifies the Front End Node                                                                               |
| `tube`             | identifies the pair of digitizers connected to the tube                                                     |
| `a`                | is the integrated voltage output of the first digitizer                                                     |
| `b`                | is the integrated voltage output of the second digitizer                                                    |
| `tof`              | is the time-of-flight of the neutron<br>default: `_particle->t`, but any `USER_VARS` value is valid         |
| `ip`               | the resolvable domain name or IP address of the EFU which will receive the packets                          |
| `port`             | the EFU event-packet UDP port, 9000 by default to match the EFU default                                     |
| `command_port`     | the EFU command TCP port, 10800 by default to match the EFU default                                         |
| `broadcast`        | flag to control if event packets are sent, on by default                                                    |
| `pulse_rate`       | the reference time pulse rate, used in calculating EFU-required time stamps                                 |
| `noisy`            | flag to control if otherwise-dropped events should be replaced by noise, off by default                     |
| `noise_level`      | if `noisy`, *how* noisy &mdash; fractional probability of noise: 0.1 by default                             |
| `event_mode`       | `"p"` for `rand_poisson(_particle->p)`, `"pp"` for p<sup>2</sup>                                            |
| `receiver`         | the path to the EFU executable, required if it is to be started locally                                     |
| `options`          | command-line options used when starting the EFU                                                             |
| `filename`         | filename for dumped EFU-received events if EFU started and output requested, `NAME_CURRENT_COMP` by default |
| `hdf5_output`      | flag to control dumping EFU-received events if EFU started, on by default                                   |
| `start_receiver`   | flag to control whether the local EFU should be started, on by default                                      |
| `verbose`          | enumerated value to control `STDOUT` printing: -1=silent, 0=errors, 1=warnings, 2=info, 3=details           |
| `ess_readout_type` | integer identifying simulated ESS readout &mdash; BIFROST: 0x34 (dec 52), CSPEC: 0x40 (dec 64)              |

## Parameters required by the Event Formation Unit
In order to identify the detector element of a <sup>3</sup>He tube which records a neutron event, the EFU
requires two integers to identify the *position* along the tube where the event occurred, `AmpA` and `AmpB`.
In units of the length of the tube, the event position is then $\frac{A}{A+B}$ relative to the 'B' end of the tube; 
or $\frac{A-B}{A+B}$ relative to the center in units of half the tube length.

The EFU also requires three integers to identify in *which* tube the event occurred: the `RingId` identifies the
readout cables connected to the Front End Node; the `FENId` identifies the Front End Node (indexed from 0, with the
maximum index depending on the number of readout Rings required for an instrument); and the `TubeId` identifies which
*pair* of digitizers on the Ring produced the two integrated amplitude signals.

These parameters are read from the `USER_VARS` contained in the `_particle` `struct` which *should* be set appropriately
earlier in the McStas instrument. The component identifies the `struct` parameters *by name* and requires the names
as input component parameters according to the following table:

| Component parameter | EFU parameter |
|---------------------|---------------|
| `ring`              | `RingId`      |
| `fen`               | `FENId`       |
| `tube`              | `TubeId`      |
| `a`                 | `AmpA`        |
| `b`                 | `AmpB`        |

Finally, the EFU requires the event time, the most-recent reference time, and the previous reference time to determine
the *relative* event time which is ultimately stored in a NeXus file.
These times must be encoded as two integers: the `high` time, which is the number of whole seconds since the UNIX epoch,
`1970-01-01T00:00Z`, and the `low` time, which is the number of `88052499 Hz` clock ticks since the last whole second.  

The reference times are determined automatically based on the component parameter `pulse_rate` and the running time of
the McStas simulation.
The event time is set to the most-recent reference time plus neutron time-of-flight, which is read from `_particle->t`
by default but can be overridden by setting the component parameter `tof`.

## Installation

After cloning this repository, use CMake to configure, build, and install the shared C library.

To install the library and configuration-query tool within the current user's home directory, e.g., execute 
```bash
git clone https://github.com/g5t/mcstas-readout-master.git
cmake -S mcstas-readout-master -B mcstas-readout-master-build -DCMAKE_INSTALL_PATH=~
cmake --build mcstas-readout-master-build --target install
```

The component file `Readout.comp` can then be used in a McStas 3.2+ instrument by copying the file from the cloned
repository or, e.g., producing a soft-link to its location.

## Automatic EFU start/stop and HDF5 raw event file
Performing scans in McStas which produce raw EFU-captured event files will be necessary to simulate some calibration procedures.
The Readout McStas component can spawn a second process and start the EFU (or other packet receiver) before the simulation of one scan point,
and stop it at the end of the scan point.
By default the output HDF5 files are placed within the McStas output directory, and can therefore be associated with the scan points directly.

## MPI Support
McStas can be run on any number of MPI workers. If the `Readout` component is run in MPI all nodes should have network
access to the host running the EFU. If the EFU is controlled by the `Readout` component, it will be started only by the
master MPI node.
