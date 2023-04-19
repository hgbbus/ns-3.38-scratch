#!/usr/bin/env bash
#

# Script directories
NS3DIR="../.."
CWD="$PWD"
#OUTDIR="$CWD/out"
#if [ ! -d "$OUTDIR" ]; then
#  mkdir -p "$OUTDIR"
#fi

# Script name is extracted from its directory name
THIS_SCRIPT=$(basename -- $CWD)

# If you need to change simulation script parameters, 
# this is the place to do it.
PROG="$THIS_SCRIPT --pingOn --nFTPs=5"

# Run script using 'waf'
cd $NS3DIR > /dev/null
NS3VERSION=$(head -n 1 VERSION)
if [[ "$NS3VERSION" > "3.35" ]]; then
  ./ns3 run "$PROG" --cwd="$OUTDIR"
else
  ./waf -j4 --cwd="$OUTDIR" --run "$PROG"
fi
cd - > /dev/null
