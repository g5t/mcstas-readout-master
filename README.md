# McStas Readout Master for BIFROST

After installing [McStas 3.x](https://mcstas.org), compile and run the test instrument via

```bash
mcrun broadcaster.instr -n 100 --no-output
```

Compile the shared library with, e.g., 
```bash
cmake -S . -B build
cmake --build build
```

## Automatic EFU start/stop and HDF5 raw event file
Performing scans in McStas which produce raw EFU-captured event files will be necessary to simulate some calibration procedures.
The Event_broacaster can now spawn a second process and start the EFU (or other packet receiver) before the simulation of one scan point,
and stop it at the end of the scan point.
By also setting an output HDF5 filename within the McStas output directory, the raw files can be associated with the scan points directly.
