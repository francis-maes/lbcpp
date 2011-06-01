/*-----------------------------------------.---------------------------------.
| Filename: RunBoincWorkUnit.cpp           | A program to launch work units  |
| Author  : Arnaud Schoofs                 | on BOINC                        |
| Started : 16/12/2010 12:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

// TODO : merge RunBoincWorkUnit and RunWorkUnit

#include <lbcpp/Execution/WorkUnit.h>
#include <lbcpp/Execution/ExecutionTrace.h>
#include <lbcpp/library.h>
#include <iostream>
#include <fstream>
using namespace lbcpp;

/**
 * Callback used to write BOINC progression file
 */
// FIXME : general mechanism to write global progression
class BoincProgressionExecutionCallback : public ExecutionCallback
{
public:
  virtual void progressCallback(const ProgressionStatePtr& progression)
  {
    // FIXME : hard coded for ProteinLearner !!
    if (progression->getUnit() == T("Learning Iterations")) {
      writeProgressionFile(progression->getNormalizedValue());
    }
  }
  
  virtual void preExecutionCallback(const ExecutionStackPtr& stack,
                                    const String& description, const WorkUnitPtr& workUnit)
  {
    // appelé lorsque l'on rentre dans un sous-scope
  }
  
  virtual void postExecutionCallback(const ExecutionStackPtr& stack,
                                     const String& description, const WorkUnitPtr& workUnit, const
                                     Variable& result)
  {
    // appelé lorsque l'on sort d'un sous-scope
  }
  
private:  
  void writeProgressionFile(double globalProgression)
  {
    std::ofstream progressionFile("fractiondone", std::ios::trunc);
    if (progressionFile.is_open())
    {
      progressionFile << globalProgression;
      progressionFile.close();
    }
  }
};


void usage()
{
  std::cerr << "Usage: RunBoincWorkUnit [--numThreads n --library lib --trace file.trace --traceAutoSave 60 --projectDirectory path] WorkUnitFile.xml" << std::endl;
  std::cerr << "Usage: RunBoincWorkUnit [--numThreads n --library lib --trace file.trace --traceAutoSave 60 --projectDirectory path] WorkUnitName WorkUnitArguments" << std::endl;
  std::cerr << "  --numThreads : the number of threads to use. Default value: n = the number of cpus." << std::endl;
  std::cerr << "  --library : add a dynamic library to load." << std::endl;
  std::cerr << "  --trace : output file to save the execution trace." << std::endl;
  std::cerr << "  --traceAutoSave : the interval in seconds between two execution trace auto-saves." << std::endl;
  std::cerr << "  --projectDirectory : project directory where find files." << std::endl;
}

bool parseTopLevelArguments(ExecutionContext& context, int argc, char** argv, std::vector<String>& remainingArguments,
                            size_t& numThreads, File& traceOutputFile, double& traceAutoSave, File& projectDirectory)
{
  numThreads = (size_t)juce::SystemStats::getNumCpus();
  traceAutoSave = 0.0; // no auto save
  
  remainingArguments.reserve(argc - 1);
  for (int i = 1; i < argc; ++i)
  {
    String argument = argv[i];
    if (argument == T("--numThreads"))
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
      File dynamicLibraryFile = File::getCurrentWorkingDirectory().getChildFile(argv[i]);
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
      traceOutputFile = File::getCurrentWorkingDirectory().getChildFile(argv[i]);
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
      traceAutoSave = String(argv[i]).getIntValue();
    }
    else if (argument == T("--projectDirectory"))
    {
      ++i;
      if (i == argc)
      {
        context.errorCallback(T("Invalid Syntax"));
        return false;
      }
      projectDirectory = File::getCurrentWorkingDirectory().getChildFile(argv[i]);
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
  Variable result = context.run(workUnit);
  return !result.isBoolean() || result.getBoolean();
}

bool runWorkUnitFromFile(ExecutionContext& context, const File& file)
{
  ObjectPtr object = Object::createFromFile(context, file);
  return object && checkIsAWorkUnit(context, object) && runWorkUnit(context, object.staticCast<WorkUnit>());
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
  size_t numThreads;
  File traceOutputFile;
  double traceAutoSave;
  File projectDirectory;
  if (!parseTopLevelArguments(defaultExecutionContext(), argc, argv, arguments, numThreads, traceOutputFile, traceAutoSave, projectDirectory))
  {
    std::cerr << "Could not parse top level arguments." << std::endl;
    usage();
    return 1;
  }

  // replace default context
  if (projectDirectory == File::nonexistent)
    projectDirectory = File::getCurrentWorkingDirectory();
  ExecutionContextPtr context = (numThreads == 1 ? singleThreadedExecutionContext(projectDirectory) : multiThreadedExecutionContext(numThreads, projectDirectory));
  setDefaultExecutionContext(context);
  
  // console callback
  context->appendCallback(consoleExecutionCallback());
  
  // add "make trace" callback
  ExecutionCallbackPtr makeTraceCallback;
  ExecutionTracePtr trace;
  if (traceOutputFile != File::nonexistent)
  {
    trace = new ExecutionTrace(context->toString());
    if (traceAutoSave == 0.0)
      makeTraceCallback = makeTraceExecutionCallback(trace);
    else
      makeTraceCallback = makeAndAutoSaveTraceExecutionCallback(trace, traceAutoSave, traceOutputFile);
    context->appendCallback(makeTraceCallback);
  }

  context->appendCallback(new BoincProgressionExecutionCallback());
  
  // run work unit from file
  int result = 0;
  jassert(arguments.size());
  File firstArgumentAsFile = File::getCurrentWorkingDirectory().getChildFile(arguments[0]);
  if (arguments.size() == 1 && firstArgumentAsFile.existsAsFile())
    result = runWorkUnitFromFile(*context, firstArgumentAsFile) ? 0 : 1;
  else
  {
    jassertfalse;
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

int main(int argc, char** argv)
{
  lbcpp::initialize(argv[0]);
  int exitCode = mainImpl(argc, argv);
  lbcpp::deinitialize();
  return exitCode;
}
