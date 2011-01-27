/*-----------------------------------------.---------------------------------.
| Filename: Command.cpp                    | Network Command                 |
| Author  : Julien Becker                  |                                 |
| Started : 02/12/2010 18:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "NetworkCommand.h"

using namespace lbcpp;


namespace lbcpp
{

bool ClientNetworkCommand::runCommand(ExecutionContext& context, NetworkCommandPtr network)
{
  ClientNetworkContextPtr clientNetwork = network.staticCast<ClientNetworkContext>();
  if (!clientNetwork)
  {
    context.errorCallback(T("GetWorkUnitNetworkCommand::runCommand"),
                          T("Must be execute in a ClientNetworkContext only"));
    return false;
  }
  return runCommand(context, clientNetwork);
}

bool ServerNetworkCommand::runCommand(ExecutionContext& context, NetworkContextPtr network)
{
  ServerNetworkContextPtr serverNetwork = network.staticCast<ServerNetworkContext>();
  if (!serverNetwork)
  {
    context.errorCallback(T("GetWorkUnitStatusNetworkCommand::runCommand"),
                          T("Must be execute in a ServerNetworkContext only"));
    return false;
  }
  return runCommand(context, serverNetwork);
}
  

}; /* namespace lbcpp */
