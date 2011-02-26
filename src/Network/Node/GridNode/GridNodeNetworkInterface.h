/*-----------------------------------------.---------------------------------.
| Filename: GridNodeNetworkInterface.h     | Grid Node Network Interface     |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 19:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRID_NODE_NETWORK_INTERFACE_H_
# define LBCPP_GRID_NODE_NETWORK_INTERFACE_H_

# include "../NetworkRequest.h"
# include "../NodeNetworkInterface.h"

namespace lbcpp
{

class GridNodeNetworkInterface : public NodeNetworkInterface
{
public:
  GridNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName)
    : NodeNetworkInterface(context, client, nodeName) {}
  GridNodeNetworkInterface() {}

  // input : containerClass(networkRequestClass)
  // return: containerClass(stringType)
  // special return value in case of error: T("Error")
  virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests) = 0;
  // return: containerClass(networkResponse)
  virtual ContainerPtr getFinishedExecutionTraces() = 0;
  // Normally not send on network
  virtual void removeExecutionTraces(ContainerPtr networkResponses) = 0;
};

typedef ReferenceCountedObjectPtr<GridNodeNetworkInterface> GridNodeNetworkInterfacePtr;

extern ClassPtr gridNodeNetworkInterfaceClass;

class ClientGridNodeNetworkInterface : public GridNodeNetworkInterface
{
public:
  ClientGridNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName)
    : GridNodeNetworkInterface(context, client, nodeName) {}
  ClientGridNodeNetworkInterface() {}

  virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests);
  virtual ContainerPtr getFinishedExecutionTraces();
  virtual void removeExecutionTraces(ContainerPtr networkResponses);
  
  /* NetworkInterface */
  void closeCommunication();
};

class SgeGridNodeNetworkInterface : public GridNodeNetworkInterface
{
public:
  SgeGridNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName);
  SgeGridNodeNetworkInterface() {}

  virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests);
  virtual ContainerPtr getFinishedExecutionTraces();
  virtual void removeExecutionTraces(ContainerPtr networkResponses);

protected:
  NetworkResponsePtr getNetworkResponse(const String& identifier)
  {
    File f = context.getFile(T("Traces/") + identifier + T(".trace"));
    if (!f.exists())
      return new NetworkResponse(context, identifier);
    ExecutionTracePtr trace = ExecutionTrace::createFromFile(context, f);
    return new NetworkResponse(context, identifier, trace);
  }

  File getRequestFile(NetworkRequestPtr request)
    {return context.getFile(T("Requests/") + request->getIdentifier() + T(".request"));}

  File getWaitingFile(NetworkRequestPtr request)
    {return context.getFile(T("Waiting/") + request->getIdentifier() + T(".workUnit"));}

  File getFinishDirectory()
    {return context.getProjectDirectory().getChildFile(T("Finished"));}

  void createDirectoryIfNotExists(const String& directory)
  {
    File f = context.getFile(directory);
    if (!f.exists())
      f.createDirectory();
  }
};

class BoincGridNodeNetworkInterface : public GridNodeNetworkInterface
{
public:
  BoincGridNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName)
    : GridNodeNetworkInterface(context, client, nodeName) {}
  BoincGridNodeNetworkInterface() {}

  virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests)
    {jassertfalse; return ContainerPtr();}
  virtual ContainerPtr getFinishedExecutionTraces()
    {jassertfalse; return ContainerPtr();}
  virtual void removeExecutionTraces(ContainerPtr networkResponses)
    {jassertfalse;}
};

};

#endif //!LBCPP_GRID_NODE_NETWORK_INTERFACE_H_
