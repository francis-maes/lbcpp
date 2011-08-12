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
# include <lbcpp/Network/NetworkClient.h>
# include <lbcpp/Network/NetworkServer.h>
# include <lbcpp/Network/NetworkMessage.h>

namespace lbcpp
{

/** ServerWorkUnit **/
class ServerWorkUnit : public WorkUnit
{
public:
  ServerWorkUnit(size_t port = 1664)
    : port(port) {}

  Variable run(ExecutionContext& context)
  {
    NetworkServerPtr server = managerNetworkServer(context, new Manager(context));
    return server->startServer(port);
  }

protected:
  friend class ServerWorkUnitClass;

  size_t port;
};

/** GridWorkUnit **/
class GridWorkUnit : public WorkUnit
{
public:
  GridWorkUnit() : clientType(T("local")), gridName(T("localGrid"))
                 , managerHostName(T("localhost")), managerPort(1664) {}

  Variable run(ExecutionContext& context)
  {
    GridNetworkClientPtr client;
    if (clientType == T("local"))
      client = localGridNetworkClient(context);
    else if (clientType == T("sge"))
      client;// = sgeGridNetworkClient(context);
    else
    {
      context.errorCallback(T("GridWorkUnit::run"), T("Unknown GridNetworkClient nammed: ") + clientType.quoted());
      return false;
    }

    client->startClient(managerHostName, managerPort);
    client->askForWorkUnits(gridName);
    client->sendFinishedTraces();
    client->waitResponses(300000); // 5"
    client->closeCommunication();

    return true;
  }

protected:
  friend class GridWorkUnitClass;

  String clientType;
  String gridName;

  String managerHostName;
  size_t managerPort;
};

}; /* namespace */

#endif // !LBCPP_NETWORK_WORK_UNIT_H_
