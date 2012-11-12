/*-----------------------------------------.---------------------------------.
| Filename: RunWorkUnit.cpp                | A program to launch work units  |
| Author  : Francis Maes                   |                                 |
| Started : 16/12/2010 12:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Execution/WorkUnit.h>
#include <lbcpp/Execution/ExecutionTrace.h>
#include <lbcpp/library.h>
using namespace lbcpp;

void usage()
{
  std::cerr << "Usage: RunWorkUnit [--numThreads n --library lib --trace file.trace --traceAutoSave 60 --projectDirectory path] WorkUnitFile.xml" << std::endl;
  std::cerr << "Usage: RunWorkUnit [--numThreads n --library lib --trace file.trace --traceAutoSave 60 --projectDirectory path] WorkUnitName WorkUnitArguments" << std::endl;
  std::cerr << "  --numThreads : the number of threads to use. Default value: n = the number of cpus." << std::endl;
  std::cerr << "  --library : add a dynamic library to load." << std::endl;
  std::cerr << "  --trace : output file to save the execution trace." << std::endl;
  std::cerr << "  --traceAutoSave : the interval in seconds between two execution trace auto-saves." << std::endl;
  std::cerr << "  --projectDirectory : project directory where find files." << std::endl;
}

bool parseTopLevelArguments(ExecutionContext& context, int argc, char** argv, std::vector<string>& remainingArguments,
                            size_t& numThreads, juce::File& traceOutputFile, double& traceAutoSave, juce::File& projectDirectory)
{
  numThreads = 1;//(size_t)juce::SystemStats::getNumCpus();
  traceAutoSave = 0.0; // no auto save
  
  remainingArguments.reserve(argc - 1);
  for (int i = 1; i < argc; ++i)
  {
    string argument = argv[i];
    if (argument == T("--numThreads"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      int n = string(argv[i]).getIntValue();
      if (n < 1)
      {
        context.errorCallback(T("Invalid number of threads"));
        return false;
      }
      numThreads = (size_t)n;
    }
    else if (argument == T("--library"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      juce::File dynamicLibraryFile = juce::File::getCurrentWorkingDirectory().getChildFile(argv[i]);
      if (!lbcpp::importLibraryFromFile(defaultExecutionContext(), dynamicLibraryFile))
        return false;
    }
    else if (argument == T("--trace"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      traceOutputFile = juce::File::getCurrentWorkingDirectory().getChildFile(argv[i]);
      if (traceOutputFile.exists())
        traceOutputFile.deleteFile();
    }
    else if (argument == T("--traceAutoSave"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      traceAutoSave = string(argv[i]).getIntValue();
    }
    else if (argument == T("--projectDirectory"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      projectDirectory = juce::File::getCurrentWorkingDirectory().getChildFile(argv[i]);
      if (!projectDirectory.isDirectory())
      {
        context.errorCallback(T("Invalid Project Directory"));
        return false;
      }
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

bool runWorkUnit(ExecutionContext& context, WorkUnitPtr workUnit)
{
  BooleanPtr result = context.run(workUnit).dynamicCast<Boolean>();
  return !result || result->get();
}

bool runWorkUnitFromFile(ExecutionContext& context, const juce::File& file)
{
  ObjectPtr object = Object::createFromFile(context, file);
  return object && checkIsAWorkUnit(context, object) && runWorkUnit(context, object.staticCast<WorkUnit>());
}

bool runWorkUnitFromArguments(ExecutionContext& context, const string& workUnitClassName, const std::vector<string>& arguments)
{
  // find the work unit class
  ClassPtr type = typeManager().getType(context, workUnitClassName);
  if (!type)
    return false;

  // create the work unit
  ObjectPtr object = Object::create(type);
  if (!object || !checkIsAWorkUnit(context, object))
    return false;

  // look if usage is requested
  const WorkUnitPtr& workUnit = object.staticCast<WorkUnit>();
  for (size_t i = 0; i < arguments.size(); ++i)
    if (arguments[i] == T("--help"))
    {
      context.informationCallback(workUnit->getUsageString());
      return false;
    }

  // parse arguments and run work unit
  return workUnit->parseArguments(context, arguments) && runWorkUnit(context, workUnit);
}

int mainImpl(int argc, char** argv)
{
  if (argc == 1)
  {
    usage();
    return 1;
  }

  // load dynamic libraries
  lbcpp::importLibrariesFromDirectory(juce::File::getCurrentWorkingDirectory());
  if (juce::File::isAbsolutePath(argv[0]))
  {
    juce::File executablePath = juce::File(argv[0]).getParentDirectory();
    if (executablePath != juce::File::getCurrentWorkingDirectory())
      lbcpp::importLibrariesFromDirectory(executablePath);
  }

  // parse top level arguments
  std::vector<string> arguments;
  size_t numThreads;
  juce::File traceOutputFile;
  double traceAutoSave;
  juce::File projectDirectory;
  if (!parseTopLevelArguments(defaultExecutionContext(), argc, argv, arguments, numThreads, traceOutputFile, traceAutoSave, projectDirectory))
  {
    std::cerr << "Could not parse top level arguments." << std::endl;
    usage();
    return 1;
  }

  // replace default context
  if (projectDirectory == juce::File::nonexistent)
    projectDirectory = juce::File::getCurrentWorkingDirectory();
  ExecutionContextPtr context = (numThreads == 1 ? singleThreadedExecutionContext(projectDirectory) : multiThreadedExecutionContext(numThreads, projectDirectory));
  setDefaultExecutionContext(context);
  context->appendCallback(consoleExecutionCallback());
  // add "make trace" callback
  ExecutionCallbackPtr makeTraceCallback;
  ExecutionTracePtr trace;
  if (traceOutputFile != juce::File::nonexistent)
  {
    trace = new ExecutionTrace(context->toString());
    if (traceAutoSave == 0.0)
      makeTraceCallback = makeTraceExecutionCallback(trace);
    else
      makeTraceCallback = makeAndAutoSaveTraceExecutionCallback(trace, traceAutoSave, traceOutputFile);
    context->appendCallback(makeTraceCallback);
  }

  // run work unit either from file or from arguments
  int result = 0;
  jassert(arguments.size());
  juce::File firstArgumentAsFile = juce::File::getCurrentWorkingDirectory().getChildFile(arguments[0]);
  if (arguments.size() == 1 && firstArgumentAsFile.existsAsFile())
    result = runWorkUnitFromFile(*context, firstArgumentAsFile) ? 0 : 1;
  else
  {
    string workUnitClassName = arguments[0];
    arguments.erase(arguments.begin());
    result = runWorkUnitFromArguments(*context, workUnitClassName, arguments) ? 0 : 1;
  }

  // save trace
  if (makeTraceCallback)
  {
    context->removeCallback(makeTraceCallback);
    context->informationCallback(T("Saving execution trace into ") + traceOutputFile.getFullPathName());
    trace->saveToFile(*context, traceOutputFile);
  }
  return result;
}

namespace lbcpp
{
  extern LibraryPtr lbCppMLLibrary();
};

int main(int argc, char** argv)
{
  lbcpp::initialize(argv[0]);
  lbcpp::importLibrary(lbCppMLLibrary());
  int exitCode = mainImpl(argc, argv);
  lbcpp::deinitialize();
  return exitCode;
}
