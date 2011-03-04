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
# include "../Node/NodeNetworkInterface.h"
# include "../Node/ManagerNode/ManagerNodeNetworkInterface.h"
# include "../Node/GridNode/GridNodeNetworkInterface.h"

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

  void serverCommunication(ExecutionContext& context, ManagerNodeNetworkInterfacePtr interface) const;
  void clientCommunication(ExecutionContext& context, GridNodeNetworkInterfacePtr interface);
};

class GridWorkUnit : public WorkUnit
{
public:
  GridWorkUnit(const String& gridName, const String& gridEngine, const String& hostName, size_t port)
  : gridName(gridName), gridEngine(gridEngine), hostName(hostName), port(port) {}
  GridWorkUnit() : port(1664) {}

  virtual Variable run(ExecutionContext& context);

protected:
  friend class GridWorkUnitClass;

  String gridName;
  String gridEngine;
  String hostName;
  size_t port;
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
