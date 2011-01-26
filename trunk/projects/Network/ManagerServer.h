#include <lbcpp/lbcpp.h>
#include "NetworkCommand.h"

namespace lbcpp
{

class WorkUnitManager;

class Job : public Object
{
public:
  Job(WorkUnitPtr workUnit, juce::int64 workUnitId, String client) 
    : workUnit(workUnit), workUnitId(workUnitId), client(client), status(T("WaitingForAServer")) {}
  Job() {}
  
  WorkUnitPtr getWorkUnit() const
    {return workUnit;}
  
  juce::int64 getWorkUnitId() const
    {return workUnitId;}
  
protected:
  friend class WorkUnitManager;
  
  WorkUnitPtr workUnit;
  juce::int64 workUnitId;
  String client;
  String server;
  String status;
};

typedef ReferenceCountedObjectPtr<Job> JobPtr;

class WorkUnitManager : public Object
{
public:
  WorkUnitManager() : lastWorkUnitId(0) {}
  
  juce::int64 pushWorkUnit(WorkUnitPtr workUnit, const String& client)
  {
    juce::int64 workUnitId = generateWorkUnitId();
    waitingJobs[workUnitId] = new Job(workUnit, workUnitId, client);
    return workUnitId;
  }
  
  void setStatus(JobPtr job, const String& status)
  {
    job->status = status;
    
    if (status == T("Finished"))
    {
      waitingJobs.erase(job->getWorkUnitId());
      inProgressJobs.erase(job->getWorkUnitId());
      finishedJobs[job->getWorkUnitId()] = job;
    }
  }
  
  void jobSubmittedTo(JobPtr job, const String& server)
  {
    job->status = T("JobSubmittedToServer: ") + server;
    job->server = server;
    
    waitingJobs.erase(job->getWorkUnitId());
    inProgressJobs[job->getWorkUnitId()] = job;
  }
  
  std::vector<JobPtr> jobsAssignedTo(String server) const
  {
    std::vector<JobPtr> res;
    for (std::map<juce::int64, JobPtr>::const_iterator it = inProgressJobs.begin(); it != inProgressJobs.end(); ++it)
      if (it->second->server == server)
        res.push_back(it->second);
    return res;
  }
  
  std::vector<JobPtr> getWaitingJobs() const
  {
    std::vector<JobPtr> res;
    for (std::map<juce::int64, JobPtr>::const_iterator it = waitingJobs.begin(); it != waitingJobs.end(); ++it)
      res.push_back(it->second);
    return res;
  }

protected:
  //File workUnitsDirectory;
  juce::int64 lastWorkUnitId;

  std::map<juce::int64, JobPtr> waitingJobs;
  std::map<juce::int64, JobPtr> inProgressJobs;
  std::map<juce::int64, JobPtr> finishedJobs;

  juce::int64 generateWorkUnitId()
  {
    juce::int64 res = Time::currentTimeMillis();
    if (res != lastWorkUnitId)
    {
      lastWorkUnitId = res;
      return res;
    }
    juce::Thread::sleep(1);
    return generateWorkUnitId();
  }
};

typedef ReferenceCountedObjectPtr<WorkUnitManager> WorkUnitManagerPtr;

class ManagerServer : public WorkUnit
{
public:
  ManagerServer() : port(1664), workUnitManager(new WorkUnitManager()) {}

  bool run(ExecutionContext& context)
  {
    NetworkServerPtr server = new NetworkServer(context);
    if (!server->startServer(port))
    {
      context.errorCallback(T("WorkUnitManagerServer::run"), T("Not able to open port ") + String(port));
      return false;
    }

    while (true)
    {
      NetworkClientPtr client = server->acceptClient(INT_MAX);
      if (!client)
        continue;
      String hostname = client->getConnectedHostName();
      context.informationCallback(hostname, T("connected"));

      /* Type of client */
      client->sendVariable(new GetConnectionTypeNetworkCommand());
      String clientType;
      if (!client->receiveString(10000, clientType))
      {
        context.warningCallback(hostname, T("Fail - Client type"));
        closeConnection(context, client);
        continue;
      }
      context.informationCallback(hostname, T("Connection type: ") + clientType);

      /* Blablating */
      if (clientType == T("Server"))
        serverCommunication(context, client);
      if (clientType == T("Client"))
        clientCommunication(context, client);

      closeConnection(context, client);
    }
  }

protected:
  int port;
  WorkUnitManagerPtr workUnitManager;
  
  void closeConnection(ExecutionContext& context, NetworkClientPtr client)
  {
    /* Terminate the connection */
    if (client->isConnected())
    {
      client->sendVariable(new CloseConnectionNetworkCommand());
      client->stopClient();
    }
    context.informationCallback(client->getConnectedHostName(), T("disconnected"));
  }

  bool clientCommunication(ExecutionContext& context, NetworkClientPtr client)
  {
    String hostname = client->getConnectedHostName();

    /* Get client identifier */
    client->sendVariable(new GetIdentityNetworkCommand());
    String identity;
    if (!client->receiveString(10000, identity))
    {
      context.warningCallback(hostname, T("Fail - Identity"));
      return false;
    }
    context.informationCallback(hostname, T("Name: ") + identity);

    while (true)
    {
      /* Ask and Wait for a job */
      client->sendVariable(new GetWorkUnitNetworkCommand());
      WorkUnitPtr workUnit;
      if (!client->receiveObject<WorkUnit>(10000, workUnit))
      {
        context.warningCallback(hostname, T("Fail - GetWorkUnit"));
        return false;
      }

      if (!workUnit)
        break;

      /* Generate an identifier */
      juce::int64 workUnitId = workUnitManager->pushWorkUnit(workUnit, identity);
      /* Return the identifier */
      client->sendVariable(String(workUnitId));

      context.informationCallback(hostname, T("WorkUnit ID: ") + String(workUnitId));
    }

    return true;
  }
  
  bool serverCommunication(ExecutionContext& context, NetworkClientPtr client)
  {
    String hostname = client->getConnectedHostName();

    /* Get server identifier */
    client->sendVariable(new GetIdentityNetworkCommand());
    String identity;
    if (!client->receiveString(10000, identity))
    {
      context.warningCallback(hostname, T("Fail - Identity"));
      return false;
    }
    context.informationCallback(hostname, T("Name: ") + identity);

    /* Get status of jobs submitted to the server */
    std::vector<JobPtr> assignedJobs = workUnitManager->jobsAssignedTo(identity);
    for (size_t i = 0; i < assignedJobs.size(); ++i)
    {
      client->sendVariable(new GetWorkUnitStatusNetworkCommand(assignedJobs[i]->getWorkUnitId()));
      String status;
      if (!client->receiveString(10000, status))
      {
        context.warningCallback(hostname, T("Fail - Status of work unit: ") + String(assignedJobs[i]->getWorkUnitId()));
        return false;
      }
      
      workUnitManager->setStatus(assignedJobs[i], status);
      
      if (status == T("Finished"))
      {
        client->sendVariable(new SendTraceNetworkCommand(assignedJobs[i]->getWorkUnitId()));
        Variable trace;
        if (!client->receiveVariable(10000, trace))
        {
          context.warningCallback(hostname, T("Fail - Trace of work unit: ") + String(assignedJobs[i]->getWorkUnitId()));
          return false;
        }
        
        trace.saveToFile(context, File::getCurrentWorkingDirectory().getChildFile(String(assignedJobs[i]->getWorkUnitId())));
      }
      
      if (status == T("IDontHaveThisWorkUnit"))
      {
        client->sendVariable(new PushWorkUnitNetworkCommand(assignedJobs[i]->getWorkUnitId(), assignedJobs[i]->getWorkUnit()));
        context.informationCallback(hostname, T("WorkUnit ") + String(assignedJobs[i]->getWorkUnitId()) + T(" has been sent"));
      }
    }

    /* Send not yet assigned job to server */
    std::vector<JobPtr> waitingJobs = workUnitManager->getWaitingJobs();
    if (!waitingJobs.size())
      return true;

    SystemResourcePtr resource;
    for (size_t i = 0; i < waitingJobs.size(); ++i)
    {
      /* Get available ressouces */
      if (!resource)
      {
        client->sendVariable(new GetSystemResourceNetworkCommand());
        if (!client->receiveObject<SystemResource>(10000, resource))
        {
          context.warningCallback(hostname, T("Fail - SystemResource"));
          return false;
        }
      }
      /* Check if ressouces compatible */
      if (resource->isSufficient(waitingJobs[i]->getWorkUnit()))
      {
        client->sendVariable(new PushWorkUnitNetworkCommand(waitingJobs[i]->getWorkUnitId(), waitingJobs[i]->getWorkUnit()));
        workUnitManager->jobSubmittedTo(waitingJobs[i], identity);
        resource = SystemResourcePtr();
      }
    }

    return true;
  }
};

};
