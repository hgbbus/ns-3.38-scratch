#!/usr/bin/env bash
#

# This bash script is to assist simulation script development
# Do not use it for official simulation runs

# Script directories
NS3DIR="../.."
CWD="$PWD"
OUTDIR="$CWD/out"
if [ ! -d "$OUTDIR" ]; then
  mkdir -p "$OUTDIR"
fi

# Script name is extracted from its directory name
THIS_SCRIPT=$(basename -- $CWD)

# If you need to change simulation script parameters, 
# this is the place to do it.
#RUN="$THIS_SCRIPT --pingOn --tracing --nFTPs=5 --qdType=abc"
#PROG="$THIS_SCRIPT --nFTPs=1 --simTime=100 --devQSize=1000 --ns3::PfifoFastQueueDisc::MaxSize=2000p"
PROG="$THIS_SCRIPT --nFTPs=3 --simTime=10  --devQSize=100 --ns3::PfifoFastQueueDisc::MaxSize=1000p"

# Run script using either 'waf' or newer 'ns3'
cd $NS3DIR > /dev/null
NS3VERSION=$(head -n 1 VERSION)
if [[ "$NS3VERSION" > "3.35" ]]; then
  ./ns3 run "$PROG" --cwd="$OUTDIR"
else
  ./waf -j4 --cwd="$OUTDIR" --run "$PROG"
fi
cd - >/dev/null
