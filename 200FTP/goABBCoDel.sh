#!/usr/bin/env bash
#

#export 'NS_LOG=*=level_all|prefix_func|prefix_time'
#export 'NS_LOG=ABBCoDelQueueDisc=level_all|prefix_func|prefix_time:200FTP=level_all|prefix_func|prefix_time'
# To save log to a file, try: ./goABBCoDel.sh > log.out 2>&1

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
PROG="$THIS_SCRIPT --nFTPs=5 --simTime=10 --qdType=ABBCoDel -qdQSize=1000"

# Run script using either 'waf' or newer 'ns3'
cp "abb-flow-table.csv" $OUTDIR
cd $NS3DIR > /dev/null
NS3VERSION=$(head -n 1 VERSION)
if [[ "$NS3VERSION" > "3.35" ]]; then
  ./ns3 run "$PROG" --cwd="$OUTDIR"
else
  ./waf -j4 --cwd="$OUTDIR" --run "$PROG"
fi
cd - >/dev/null
