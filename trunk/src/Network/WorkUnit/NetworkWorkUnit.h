/*-----------------------------------------.---------------------------------.
| Filename: NetworkWorkUnit.h              | Network Work Unit               |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_WORK_UNIT_H_
# define LBCPP_NETWORK_WORK_UNIT_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Network/NetworkInterface.h>
# include "../Manager/ManagerNetworkInterface.h"
# include "../Grid/GridNetworkInterface.h"

namespace lbcpp
{

class ManagerWorkUnit : public WorkUnit
{
public:
  ManagerWorkUnit() : port(1664) {}
  
  virtual Variable run(ExecutionContext& context);

protected:
  friend class ManagerWorkUnitClass;

  size_t port;
  NetworkProjectFileManagerPtr fileManager;

  void serverCommunication(ExecutionContext& context, const ManagerNetworkInterfacePtr& interface, const NetworkClientPtr& client) const;
  void clientCommunication(ExecutionContext& context, const GridNetworkInterfacePtr& interface, const NetworkClientPtr& client);
  
private:
  void sendRequests(ExecutionContext& context, GridNetworkInterfacePtr interface, const NetworkClientPtr& client, const std::vector<WorkUnitNetworkRequestPtr>& requests) const; 
};

class GridWorkUnit : public WorkUnit // => ClientWorkUnit
{
public:
  GridWorkUnit(const String& gridName, const String& gridEngine, const String& hostName, size_t port)
    : gridName(gridName), gridEngine(gridEngine), hostName(hostName), port(port) {}
  GridWorkUnit() : port(1664) {}

  virtual Variable run(ExecutionContext& context);

protected:
  friend class GridWorkUnitClass;

  String gridName;    // clientHostName
  String gridEngine;  // remplacer par NetworkInterfacePtr clientInterface   // -i SGE
  String hostName;    // serverHostName
  size_t port;        // serverPort
};

class DumbWorkUnit : public WorkUnit
{
public:
  DumbWorkUnit() {}

  virtual Variable run(ExecutionContext& context);
};

class GetTraceWorkUnit : public WorkUnit
{
public:
  GetTraceWorkUnit()
    : hostName(T("monster24.montefiore.ulg.ac.be")), port(1664) {}

  virtual Variable run(ExecutionContext& context);

protected:
  friend class GetTraceWorkUnitClass;
  
  String workUnitIdentifier;
  String hostName;
  size_t port;
};

}; /* namespace */

#endif // !LBCPP_NETWORK_WORK_UNIT_H_
