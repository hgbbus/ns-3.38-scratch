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
#PROG="$THIS_SCRIPT --verbose --pcapOn --nPackets=3 --nFTPs=5"
PROG="$THIS_SCRIPT --verbose --nPackets=3 --nFTPs=5 --RngRun=xx"

# Run script using 'waf'
cd $NS3DIR > /dev/null
NS3VERSION=$(head -n 1 VERSION)
for rn in 1 2 3 4 5; do
  echo "Random run ${rn} ... ..."
  if [[ "$NS3VERSION" > "3.35" ]]; then
    ./ns3 run "${PROG/xx/${rn}}" --cwd="$OUTDIR"
  else
    ./waf -j4 --cwd="$OUTDIR" --run "${PROG/xx/${rn}}"
  fi
done
cd - > /dev/null
