#!/usr/bin/env bash
echo "Running integration tests..."
set -e
set -o pipefail
set -u
set -o errtrace
set -o functrace
set -o posix
set -o noclobber

# Allow for silent failure if mccode-antlr is not installed
if ! command -v mcstas-antlr &> /dev/null; then
    echo "mcstas-antlr not found, skipping integration tests."
    exit 100
fi
if ! command -v ./readout-config&> /dev/null; then
    echo "local readout-config not found."
    exit 1
fi
compdir=$(./readout-config --show compdir)

# Switch to a temporary directory
#tmpdir=$(mktemp -d)
#trap 'rm -rf -- "${tmpdir}"' EXIT
#cd ${tmpdir} || exit 1
# Create a temporary file for the test
temp_file="test_instrument.instr"
# And give it some content
/bin/cat <<EOF > "$temp_file"
DEFINE INSTRUMENT test_instrument(int dummy=0)
USERVARS
%{
int RING;
int FEN;
int TUBE;
int A;
int B;
double tof;
%}

TRACE
SEARCH "${compdir}"
COMPONENT origin = Arm() AT (0, 0, 0) ABSOLUTE
EXTEND
%{
RING = 0;
FEN = 0;
TUBE = 0;
A = 0;
B = 0;
tof = 0.0;
%}

COMPONENT readout = ReadoutCAEN(
  ring="RING", fen="FEN", tube="TUBE", event_mode="p", a_name="A", b_name="B", tof="tof", ip="127.0.0.1", port=9000,
  broadcast=0
  )
  AT (0, 0, 0) ABSOLUTE

COMPONENT monitor_readout = ReadoutTTLMonitor(
  ring="RING", fen="FEN", position="A", identity="TUBE", value="B", tof="tof", ip="127.0.0.1", port=9001, broadcast=0
)
  AT (0, 0, 0) ABSOLUTE

END
EOF

# Convert the test file into a C file
mcstas-antlr ${temp_file} || exit 1
# Compile the C file and run it
mcrun-antlr ${temp_file} -n 1 dummy=1 || exit 1

exit 0