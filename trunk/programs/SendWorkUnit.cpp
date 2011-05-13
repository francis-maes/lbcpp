/*-----------------------------------------.---------------------------------.
| Filename: SendWorkUnit.cpp                | A program to send work units   |
| Author  : Francis Maes                   | to an other server              |
| Started : 02/02/2011 15:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Execution/WorkUnit.h>
#include <lbcpp/Execution/ExecutionTrace.h>
#include <lbcpp/library.h>
#include <lbcpp/Network/NetworkClient.h>
#include <lbcpp/Network/NetworkInterface.h>
#include <lbcpp/Network/NetworkNotification.h>

using namespace lbcpp;

void usage()
{
  std::cerr << "Usage: SendWorkUnit --project name --from senderName --to gridName [--manager address[:port] --cpus n --memory gigabytes --time hours] WorkUnitFile.xml" << std::endl;
  std::cerr << "Usage: SendWorkUnit --project name --from senderName --to gridName [--manager address[:port] --cpus n --memory gigabytes --time hours] WorkUnitName WorkUnitArguments" << std::endl;
  std::cerr << "  --project : Name of project" << std::endl;
  std::cerr << "  --from : Name of expediter (you)." << std::endl;
  std::cerr << "  --to : Name of grid node." << std::endl;
  std::cerr << "  --manager : Address and port of the manager (default: monster24.montefiore.ulg.ac.be:1664)" << std::endl;
  std::cerr << "  --cpus : Number of required cpus to run the work unit." << std::endl;
  std::cerr << "  --memory : Number of gigabytes required to run the work unit." << std::endl;
  std::cerr << "  --time : Number of hours required to run the work unit." << std::endl;
}

bool parseTopLevelArguments(ExecutionContext& context, int argc, char** argv, std::vector<String>& remainingArguments,
                            String& projectName, String& source, String& destination,
                            String& managerHostName, size_t& managerPort,
                            size_t& requiredCpus, size_t& requiredMemory, size_t& requiredTime)
{  
  remainingArguments.reserve(argc - 1);
  for (int i = 1; i < argc; ++i)
  {
    String argument = argv[i];
    if (argument == T("--project"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      projectName = argv[i];
    }
    else if (argument == T("--from"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      source = argv[i];
    }
    else if (argument == T("--to"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      destination = argv[i];
    }
    else if (argument == T("--manager"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      String address = argv[i];
      int position = address.indexOf(T(":"));
      if (position == -1)
        managerHostName = address;
      else
      {
        managerHostName = address.substring(0, position);
        int port = address.substring(position + 1).getIntValue();
        if (port < 1)
        {
          context.errorCallback(T("Invalid number of port"));
          return false;
        }
        managerPort = (size_t)port;
      }

    }
    else if (argument == T("--cpus"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      int n = String(argv[i]).getIntValue();
      if (n < 1)
      {
        context.errorCallback(T("Invalid Number of CPUs"));
        return false;
      }
      requiredCpus = (size_t)n;
    }
    else if (argument == T("--memory"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      int n = String(argv[i]).getIntValue();
      if (n < 1)
      {
        context.errorCallback(T("Invalid Number of GigaBytes"));
        return false;
      }
      requiredMemory = (size_t)n;
    }
    else if (argument == T("--time"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      int n = String(argv[i]).getIntValue();
      if (n < 0)
      {
        context.errorCallback(T("Invalid Number of Hours"));
        return false;
      }
      requiredTime = (size_t)n;
    }
    else
      remainingArguments.push_back(argument);
  }
  
  if (remainingArguments.empty())
  {
    context.errorCallback(T("Missing arguments"));
    return false;
  }
  return true;
}

bool checkIsAWorkUnit(ExecutionContext& context, const ObjectPtr& object)
{
  if (!object->getClass()->inheritsFrom(workUnitClass))
  {
    context.errorCallback(object->getClassName() + T(" is not a WorkUnit class"));
    return false;
  }
  return true;
}

WorkUnitPtr getWorkUnitFromFile(ExecutionContext& context, const File& file)
{
  WorkUnitPtr object = WorkUnit::createFromFile(context, file);
  if (!object || !checkIsAWorkUnit(context, object))
    return WorkUnitPtr();
  return object;
}

WorkUnitPtr getWorkUnitFromArguments(ExecutionContext& context, const String& workUnitClassName, const std::vector<String>& arguments)
{
  // find the work unit class
  TypePtr type = typeManager().getType(context, workUnitClassName);
  if (!type)
    return false;
  
  // create the work unit
  ObjectPtr object = Object::create(type);
  if (!object || !checkIsAWorkUnit(context, object))
    return false;
  
  // look if usage is requested
  const WorkUnitPtr& workUnit = object.staticCast<WorkUnit>();
  for (size_t i = 0; i < arguments.size(); ++i)
    if (arguments[i] == T("-h") || arguments[i] == T("--help"))
    {
      context.informationCallback(workUnit->getUsageString());
      return WorkUnitPtr();
    }
  
  // parse arguments and run work unit
  workUnit->parseArguments(context, arguments);
  return workUnit;
}

int mainImpl(int argc, char** argv)
{
  if (argc == 1)
  {
    usage();
    return 1;
  }
  
  // load dynamic libraries
  lbcpp::importLibrariesFromDirectory(File::getCurrentWorkingDirectory());
  if (File::isAbsolutePath(argv[0]))
  {
    File executablePath = File(argv[0]).getParentDirectory();
    if (executablePath != File::getCurrentWorkingDirectory())
      lbcpp::importLibrariesFromDirectory(executablePath);
  }
  
  // parse top level arguments
  std::vector<String> arguments;
  String projectName;
  String source;
  String destination;
  String managerHostName(T("monster24.montefiore.ulg.ac.be"));
  size_t managerPort = 1664;
  size_t requiredCpus = 1;
  size_t requiredMemory = 2;
  size_t requiredTime = 10;
  
  ExecutionContextPtr context = singleThreadedExecutionContext(File::getCurrentWorkingDirectory());
  if (!parseTopLevelArguments(*context, argc, argv, arguments, projectName, source, destination, managerHostName, managerPort, requiredCpus, requiredMemory, requiredTime)
      || projectName == String::empty || source == String::empty || destination == String::empty)
  {
    usage();
    return 1;
  }

  context->appendCallback(consoleExecutionCallback());

  // run work unit either from file or from arguments
  WorkUnitPtr workUnit;
  jassert(arguments.size());
  File firstArgumentAsFile = File::getCurrentWorkingDirectory().getChildFile(arguments[0]);
  if (arguments.size() == 1 && firstArgumentAsFile.existsAsFile())
    workUnit = getWorkUnitFromFile(*context, firstArgumentAsFile);
  else
  {
    String workUnitClassName = arguments[0];
    arguments.erase(arguments.begin());
    workUnit = getWorkUnitFromArguments(*context, workUnitClassName, arguments);
  }

  if (!workUnit)
    return false;

  NetworkClientPtr client = blockingNetworkClient(*context);
  if (!client->startClient(managerHostName, managerPort))
  {
    context->errorCallback(T("SendWorkUnit::run"), T("Not connected !"));
    return false;
  }
  context->informationCallback(managerHostName, T("Connected !"));

  ManagerNetworkInterfacePtr interface = forwarderManagerNetworkInterface(*context, client, source);
  client->sendVariable(ReferenceCountedObjectPtr<NetworkInterface>(interface));

  WorkUnitNetworkRequestPtr request = new WorkUnitNetworkRequest(*context, projectName, source, destination, workUnit, requiredCpus, requiredMemory, requiredTime);
  String res = interface->pushWorkUnit(request);
  client->sendVariable(new CloseCommunicationNotification());
  client->stopClient();

  if (res == T("Error"))
  {
    context->errorCallback(T("SendWorkUnit::run"), T("Touble - We didn't correclty receive the acknowledgement"));
    return false;
  }
  request->setIdentifier(res);
  context->resultCallback(T("WorkUnitIdentifier"), request->getIdentifier());

  File f = context->getFile(projectName + T(".") + request->getIdentifier() + T(".request"));
  request->saveToFile(*context, f);
  
  return true;
}

int main(int argc, char** argv)
{
  lbcpp::initialize(argv[0]);
  int exitCode = mainImpl(argc, argv);
  lbcpp::deinitialize();
  return exitCode;
}
