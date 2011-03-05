/*-----------------------------------------.---------------------------------.
| Filename: GridNodeNetworkInterface.cpp   | Grid Node Network Interface     |
| Author  : Julien Becker, Arnaud Schoofs  |                                 |
| Started : 26/02/2011 19:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Data/Stream.h>
#include "GridNodeNetworkInterface.h"
#include "GridNodeNetworkNotification.h"

using namespace lbcpp;

/* ClientGridNodeNetworkInterface */
void ClientGridNodeNetworkInterface::closeCommunication()
{
  client->sendVariable(new CloseCommunicationNotification());
  client->stopClient();
  return;
}

ContainerPtr ClientGridNodeNetworkInterface::pushWorkUnits(ContainerPtr networkRequests)
{
  client->sendVariable(new PushWorkUnitsNotification(networkRequests));
  ContainerPtr res;
  if (!client->receiveObject<Container>(300000, res))
    context.warningCallback(client->getConnectedHostName(), T("ClientGridNodeNetworkInterface::pushWorkUnits"));
  return res;
}

ContainerPtr ClientGridNodeNetworkInterface::getFinishedExecutionTraces()
{
  client->sendVariable(new GetFinishedExecutionTraces());
  ContainerPtr res;
  if (!client->receiveObject<Container>(300000, res))
    context.warningCallback(client->getConnectedHostName(), T("ClientGridNodeNetworkInterface::getFinishedExecutionTraces"));
  else
    client->sendVariable(true);
  return res;
}

void ClientGridNodeNetworkInterface::removeExecutionTraces(ContainerPtr networkResponses)
{
  jassertfalse;
}

/* SgeGridNodeNetworkInterface */
SgeGridNodeNetworkInterface::SgeGridNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName)
  : GridNodeNetworkInterface(context, client, nodeName)
{
  createDirectoryIfNotExists(T("Requests"));
  createDirectoryIfNotExists(T("PreProcessing"));
  createDirectoryIfNotExists(T("WorkUnits"));
  createDirectoryIfNotExists(T("Finished"));
  createDirectoryIfNotExists(T("Traces"));
  createDirectoryIfNotExists(T("Jobs"));
}

ContainerPtr SgeGridNodeNetworkInterface::pushWorkUnits(ContainerPtr networkRequests)
{
  size_t n = networkRequests->getNumElements();
  VectorPtr res = vector(stringType, n);
  for (size_t i = 0; i < n; ++i)
  {
    NetworkRequestPtr request = networkRequests->getElement(i).getObjectAndCast<NetworkRequest>();
    WorkUnitPtr workUnit = request->getWorkUnit(context);
    if (!workUnit)
    {
      res->setElement(i, T("Error"));
      continue;
    }
    
    request->saveToFile(context, getRequestFile(request));
    workUnit->saveToFile(context, getWaitingFile(request));
    res->setElement(i, request->getIdentifier());
  }
  return res;
}

ContainerPtr SgeGridNodeNetworkInterface::getFinishedExecutionTraces()
{
  VectorPtr res = vector(networkResponseClass);
  StreamPtr files = directoryFileStream(context, getFinishDirectory(), T("*"));
  while (!files->isExhausted())
  {
    String identifier = files->next().getFile().getFileName();
    res->append(getNetworkResponse(identifier));
  }
  return res;
}

void SgeGridNodeNetworkInterface::removeExecutionTraces(ContainerPtr networkResponses)
{
  size_t n = networkResponses->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    String identifier = networkResponses->getElement(i).getObjectAndCast<NetworkResponse>()->getIdentifier();
    File f = context.getFile(T("Traces/") + identifier + T(".trace"));
    f.deleteFile();
    f = context.getFile(T("Requests/") + identifier + T(".request"));
    f.deleteFile();
    f = context.getFile(T("Finished/") + identifier);
    f.deleteFile();
    f = context.getFile(T("WorkUnits/") + identifier + T(".workUnit"));
    f.deleteFile();
  }
}

/* BoincGridNodeNetworkInterface */
BoincGridNodeNetworkInterface::BoincGridNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName)
: GridNodeNetworkInterface(context, client, nodeName)
{
  createDirectoryIfNotExists(T("Requests"));
  createDirectoryIfNotExists(T("Waiting"));
  createDirectoryIfNotExists(T("InProgress"));
  createDirectoryIfNotExists(T("Finished"));
  createDirectoryIfNotExists(T("Traces"));
}

ContainerPtr BoincGridNodeNetworkInterface::pushWorkUnits(ContainerPtr networkRequests)
{
  size_t n = networkRequests->getNumElements();
  VectorPtr res = vector(stringType, n);
  for (size_t i = 0; i < n; ++i)
  {
    NetworkRequestPtr request = networkRequests->getElement(i).getObjectAndCast<NetworkRequest>();
    WorkUnitPtr workUnit = request->getWorkUnit(context);
    if (!workUnit)
    {
      res->setElement(i, T("Error"));
      continue;
    }
    
    request->saveToFile(context, getRequestFile(request));
    workUnit->saveToFile(context, getWaitingFile(request));
    res->setElement(i, request->getIdentifier());
  }
  return res;
}

ContainerPtr BoincGridNodeNetworkInterface::getFinishedExecutionTraces()
{
  VectorPtr res = vector(networkResponseClass);
  StreamPtr files = directoryFileStream(context, getFinishDirectory(), T("*.workUnit"));
  while (!files->isExhausted())
  {
    String identifier = files->next().getFile().getFileNameWithoutExtension();
    res->append(getNetworkResponse(identifier));
  }
  return res;
}

void BoincGridNodeNetworkInterface::removeExecutionTraces(ContainerPtr networkResponses)
{
  size_t n = networkResponses->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    String identifier = networkResponses->getElement(i).getObjectAndCast<NetworkResponse>()->getIdentifier();
    File f = context.getFile(T("Traces/") + identifier + T(".trace"));
    f.deleteFile();
    f = context.getFile(T("Requests/") + identifier + T(".request"));
    f.deleteFile();
    f = context.getFile(T("Finished/") + identifier + T(".workUnit"));
    f.deleteFile();
  }
}
