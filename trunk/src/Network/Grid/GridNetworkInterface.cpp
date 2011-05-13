/*-----------------------------------------.---------------------------------.
| Filename: GridNetworkInterface.cpp       | Grid Network Interface         |
| Author  : Julien Becker, Arnaud Schoofs  |                                 |
| Started : 26/02/2011 19:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Data/Stream.h>
#include "GridNetworkInterface.h"
#include "GridNetworkNotification.h"

using namespace lbcpp;

/* ClientGridNetworkInterface */
void ClientGridNetworkInterface::closeCommunication()
{
  client->sendVariable(new CloseCommunicationNotification());
  client->stopClient();
  return;
}

ContainerPtr ClientGridNetworkInterface::pushWorkUnits(ContainerPtr networkRequests)
{
  client->sendVariable(new PushWorkUnitsNotification(networkRequests));
  ContainerPtr res;
  if (!client->receiveObject<Container>(300000, res))
    context.warningCallback(client->getConnectedHostName(), T("ClientGridNetworkInterface::pushWorkUnits"));
  return res;
}

ContainerPtr ClientGridNetworkInterface::getFinishedExecutionTraces()
{
  client->sendVariable(new GetFinishedExecutionTraces());
  ContainerPtr res;
  if (!client->receiveObject<Container>(300000, res))
    context.warningCallback(client->getConnectedHostName(), T("ClientGridNetworkInterface::getFinishedExecutionTraces"));
  else
    client->sendVariable(true);
  return res;
}

void ClientGridNetworkInterface::removeExecutionTraces(ContainerPtr networkResponses)
{
  jassertfalse;
}

/* SgeGridNetworkInterface */
SgeGridNetworkInterface::SgeGridNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& Name)
  : GridNetworkInterface(context, client, Name)
{
  createDirectoryIfNotExists(T("Requests"));
  createDirectoryIfNotExists(T("PreProcessing"));
  createDirectoryIfNotExists(T("WorkUnits"));
  createDirectoryIfNotExists(T("Finished"));
  createDirectoryIfNotExists(T("Traces"));
  createDirectoryIfNotExists(T("Jobs"));
}

ContainerPtr SgeGridNetworkInterface::pushWorkUnits(ContainerPtr networkRequests)
{
  size_t n = networkRequests->getNumElements();
  VectorPtr res = vector(stringType, n);
  for (size_t i = 0; i < n; ++i)
  {
    WorkUnitNetworkRequestPtr request = networkRequests->getElement(i).getObjectAndCast<WorkUnitNetworkRequest>();
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

ContainerPtr SgeGridNetworkInterface::getFinishedExecutionTraces()
{
  VectorPtr res = vector(networkResponseClass);
  StreamPtr files = directoryFileStream(context, getFinishDirectory(), T("*"));
  while (!files->isExhausted())
  {
    String identifier = files->next().getFile().getFileName();
    res->append(getExecutionTraceNetworkResponse(identifier));
  }
  return res;
}

void SgeGridNetworkInterface::removeExecutionTraces(ContainerPtr networkResponses)
{
  size_t n = networkResponses->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    String identifier = networkResponses->getElement(i).getObjectAndCast<ExecutionTraceNetworkResponse>()->getIdentifier();
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

/* BoincGridNetworkInterface */
BoincGridNetworkInterface::BoincGridNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& Name)
  : GridNetworkInterface(context, client, Name)
{
  createDirectoryIfNotExists(T("Requests"));
  createDirectoryIfNotExists(T("Waiting"));
  createDirectoryIfNotExists(T("InProgress"));
  createDirectoryIfNotExists(T("Finished"));
  createDirectoryIfNotExists(T("Traces"));
}

ContainerPtr BoincGridNetworkInterface::pushWorkUnits(ContainerPtr networkRequests)
{
  size_t n = networkRequests->getNumElements();
  VectorPtr res = vector(stringType, n);
  for (size_t i = 0; i < n; ++i)
  {
    WorkUnitNetworkRequestPtr request = networkRequests->getElement(i).getObjectAndCast<WorkUnitNetworkRequest>();
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

ContainerPtr BoincGridNetworkInterface::getFinishedExecutionTraces()
{
  VectorPtr res = vector(networkResponseClass);
  StreamPtr files = directoryFileStream(context, getFinishDirectory(), T("*.workUnit"));
  while (!files->isExhausted())
  {
    String identifier = files->next().getFile().getFileNameWithoutExtension();
    res->append(getExecutionTraceNetworkResponse(identifier));
  }
  return res;
}

void BoincGridNetworkInterface::removeExecutionTraces(ContainerPtr networkResponses)
{
  size_t n = networkResponses->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    String identifier = networkResponses->getElement(i).getObjectAndCast<ExecutionTraceNetworkResponse>()->getIdentifier();
    File f = context.getFile(T("Traces/") + identifier + T(".trace"));
    f.deleteFile();
    f = context.getFile(T("Requests/") + identifier + T(".request"));
    f.deleteFile();
    f = context.getFile(T("Finished/") + identifier + T(".workUnit"));
    f.deleteFile();
  }
}
