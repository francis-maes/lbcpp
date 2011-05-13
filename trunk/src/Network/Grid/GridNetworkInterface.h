/*-----------------------------------------.---------------------------------.
| Filename: GridNetworkInterface.h         | Grid Network Interface          |
| Author  : Julien Becker                  |                                 |
| Started : 26/02/2011 19:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRID_NETWORK_INTERFACE_H_
# define LBCPP_GRID_NETWORK_INTERFACE_H_

# include <lbcpp/Network/NetworkInterface.h>

namespace lbcpp
{

extern ClassPtr gridNetworkInterfaceClass;

class ClientGridNetworkInterface : public GridNetworkInterface
{
public:
  ClientGridNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName)
    : GridNetworkInterface(context, client, nodeName) {}
  ClientGridNetworkInterface() {}

  virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests);
  virtual ContainerPtr getFinishedExecutionTraces();
  virtual void removeExecutionTraces(ContainerPtr networkResponses);
  
  /* NetworkInterface */
  void closeCommunication();
};

class SgeGridNetworkInterface : public GridNetworkInterface
{
public:
  SgeGridNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName);
  SgeGridNetworkInterface() {}

  virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests);
  virtual ContainerPtr getFinishedExecutionTraces();
  virtual void removeExecutionTraces(ContainerPtr networkResponses);

protected:
  ExecutionTraceNetworkResponsePtr getExecutionTraceNetworkResponse(const String& identifier)
  {
    File f = context.getFile(T("Traces/") + identifier + T(".trace"));
    if (!f.exists())
      return new ExecutionTraceNetworkResponse(identifier);
    ExecutionTracePtr trace = ExecutionTrace::createFromFile(context, f);
    return new ExecutionTraceNetworkResponse(context, identifier, trace);
  }

  File getRequestFile(WorkUnitNetworkRequestPtr request)
    {return context.getFile(T("Requests/") + request->getIdentifier() + T(".request"));}

  File getWaitingFile(WorkUnitNetworkRequestPtr request)
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

class BoincGridNetworkInterface : public GridNetworkInterface
{
public:
  BoincGridNetworkInterface(ExecutionContext& context, NetworkClientPtr client, const String& nodeName);
  BoincGridNetworkInterface() {}

  virtual ContainerPtr pushWorkUnits(ContainerPtr networkRequests);
  virtual ContainerPtr getFinishedExecutionTraces();
  virtual void removeExecutionTraces(ContainerPtr networkResponses);
  
protected:
  ExecutionTraceNetworkResponsePtr getExecutionTraceNetworkResponse(const String& identifier)
  {
    File f = context.getFile(T("Traces/") + identifier + T(".trace"));
    if (!f.exists())
      return new ExecutionTraceNetworkResponse(identifier);
    ExecutionTracePtr trace = ExecutionTrace::createFromFile(context, f);
    return new ExecutionTraceNetworkResponse(context, identifier, trace);
  }
  
  File getRequestFile(WorkUnitNetworkRequestPtr request)
    {return context.getFile(T("Requests/") + request->getIdentifier() + T(".request"));}
  File getWaitingFile(WorkUnitNetworkRequestPtr request)
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

#endif //!LBCPP_GRID_NETWORK_INTERFACE_H_
