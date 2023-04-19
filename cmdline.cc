#include "ns3/core-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("CmdLineEx");

int
main (int argc, char *argv[])
{
  LogComponentEnable ("CmdLineEx", LOG_LEVEL_LOGIC);

  CommandLine cmd;
  cmd.Parse(argc, argv);
  
  return 0;
}