
#include "NetworkContext.h"
#include "NetworkCommand.h"

using namespace lbcpp;

Variable NetworkContext::run(ExecutionContext& context)
{
  /* Establishing a connection */
  client = blockingNetworkClient(context, 3);
  
  if (!client->startClient(hostname, port))
  {
    context.warningCallback(T("NetworkContext::run"), T("Connection to ") + hostname.quoted() + ("fail !"));
    client->stopClient();
    return false;
  }
  context.informationCallback(T("NetworkContext::run"), T("Connected to ") + hostname + T(":") + String(port));
  
  /* Slave mode - Execute received commands */
  while (client->isConnected() || client->hasVariableInQueue())
  {
    NetworkCommandPtr command;
    if (!client->receiveObject<NetworkCommand>(10000, command) || !command)
    {
      context.warningCallback(T("NetworkContext::run"), T("No command received"));
      return false;
    }
    
    command->runCommand(context, NetworkContextPtr(this));
  }
  return true;
}


String SgeServerNetworkContext::getWorkUnitStatus(ExecutionContext& context, juce::int64 workUnitId) const
{
  File f = workUnitDirectory.getChildFile(T("Waiting/") + String(workUnitId) + T(".workUnit"));
  if (f.exists())
    return T("WaitingOnServer");
  f = workUnitDirectory.getChildFile(T("InProgress/") + String(workUnitId) + T(".workUnit"));
  if (f.exists())
    return T("InProgress");
  f = workUnitDirectory.getChildFile(T("Finished/") + String(workUnitId) + T(".workUnit"));
  if (f.exists())
    return T("Finished");
  return T("IDontHaveThisWorkUnit");
}

void SgeServerNetworkContext::pushWorkUnit(ExecutionContext& context, juce::int64 workUnitId, WorkUnitPtr workUnit)
{
  std::cout << "Server - Work unit received: " << workUnitId << std::endl;
  
  /* Save work unit */
  File workUnitFile = workUnitDirectory.getChildFile(T("Waiting/") + String(workUnitId) + T(".workUnit"));
  workUnit->saveToFile(context, workUnitFile);
}

Variable SgeServerNetworkContext::getWorkUnitTrace(ExecutionContext& context, juce::int64 workUnitId) const
{
  File f = workUnitDirectory.getChildFile(T("Traces/") + String(workUnitId) + T(".trace"));
  if (!f.exists())
    return Variable();
  
  return Variable::createFromFile(context, f);
}
