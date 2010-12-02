/*-----------------------------------------.---------------------------------.
| Filename: Command.h                      | Network Command                 |
| Author  : Julien Becker                  |                                 |
| Started : 02/12/2010 18:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NETWORK_COMMAND_H_
# define LBCPP_NETWORK_COMMAND_H_

# include <lbcpp/lbcpp.h>
# include "NetworkClient.h"

namespace lbcpp
{

class Command : public WorkUnit
{
public:
  bool runCommand(ExecutionContext& context, NetworkClientPtr client);

  virtual bool callOnCurrentThread() const
    {return true;}
  
protected:
  NetworkClientPtr client;
};

typedef ReferenceCountedObjectPtr<Command> CommandPtr;

class SystemStats : public Object
{
public:
  size_t numCpus;
  size_t freeCpus;
  size_t memory;
  size_t freeMemory;
  
  virtual String toString() const
  {return T("SystemStats - CPUs: ") + String((int)numCpus) + T(" Memory: ") + String((int)memory);}
};

typedef ReferenceCountedObjectPtr<SystemStats> SystemStatsPtr;

/** InformationContextCommand **/
class InformationContextCommand : public Command
{
public:
  InformationContextCommand(const String& text = String::empty) : text(text) {}
  
protected:
  friend class InformationContextCommandClass;
  String text;
  
  virtual bool run(ExecutionContext& context)
  {context.informationCallback(client->getConnectedHostName(), text); return true;}
};

/** EchoCommand **/
class EchoCommand : public Command
{
public:
  EchoCommand(const String& echo = String::empty) : echo(echo) {}
  
protected:
  friend class EchoCommandClass;
  String echo;
  
  virtual bool run(ExecutionContext& context)
  {client->sendVariable(new InformationContextCommand(echo)); return true;}
};

/** SystemCommand **/
class SystemCommand : public Command
{
public:
  SystemCommand(const String& command = String::empty) : command(command) {}
  
protected:
  String command;
  
  virtual bool run(ExecutionContext& context)
  {return system(command.toUTF8()) == 0;}
};

/** SystemStatCommand **/
extern ClassPtr systemStatsClass;

class SystemStatCommand : public Command
{
public:
  SystemStatCommand() {}
  
protected:
  virtual bool run(ExecutionContext& context)
  {
    SystemStatsPtr stats = new SystemStats();
    stats->numCpus = juce::SystemStats::getNumCpus();
    stats->freeCpus = 0;
    stats->memory = juce::SystemStats::getMemorySizeInMegabytes();
    stats->freeMemory = 0;
    client->sendVariable(Variable(stats, systemStatsClass));
    return true;
  }
};

/** WorkUnitCommand **/
class WorkUnitCommand : public Command
{
public:
  WorkUnitCommand(WorkUnitPtr workUnit) : unit(workUnit) {}
  WorkUnitCommand() {}
  
  virtual bool callOnCurrentThread() const
  {return false;}
  
  virtual bool run(ExecutionContext& context)
  {return unit->run(context);}
  
protected:
  friend class WorkUnitCommandClass;
  
  WorkUnitPtr unit;
};

extern CommandPtr echoCommand(const String& echo);
extern CommandPtr informationContextCommand(const String& text);
extern CommandPtr systemStatCommand();
extern CommandPtr workUnitCommand(WorkUnitPtr unit);

}; /* namespace lbcpp */

#endif //!LBCPP_NETWORK_COMMAND_H_
