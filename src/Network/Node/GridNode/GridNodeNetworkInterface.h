/*-----------------------------------------.---------------------------------.
| Filename: GridNodeNetworkInterface.h     | Grid Node Network Interface     |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 19:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRID_NODE_NETWORK_INTERFACE_H_
# define LBCPP_GRID_NODE_NETWORK_INTERFACE_H_

# include <lbcpp/Network/NetworkInterface.h>

namespace lbcpp
{

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
      return new NetworkResponse(identifier);
    ExecutionTracePtr trace = ExecutionTrace::createFromFile(context, f);
    return new NetworkResponse(context, identifier, trace);
  }

  File getRequestFile(NetworkRequestPtr request)
    {return context.getFile(T("Requests/") + request->getIdentifier() + T(".request"));}

  File getWaitingFile(NetworkRequestPtr request)
    {return context.getFile(T("PreProcessing/") + request->getIdentifier() + T(".workUnit"));}

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
  BoincGridNodeNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName);
  BoincGridNodeNetworkInterface() {}

  virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests);
  virtual ContainerPtr getFinishedExecutionTraces();
  virtual void removeExecutionTraces(ContainerPtr networkResponses);
  
protected:
  NetworkResponsePtr getNetworkResponse(const String& identifier)
  {
    File f = context.getFile(T("Traces/") + identifier + T(".trace"));
    if (!f.exists())
      return new NetworkResponse(identifier);
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

};

#endif //!LBCPP_GRID_NODE_NETWORK_INTERFACE_H_
