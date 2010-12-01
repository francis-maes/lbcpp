
#include <lbcpp/lbcpp.h>

using namespace lbcpp;

void usage()
{
  std::cout << "Usage ..." << std::endl;
}

extern void declareProteinClasses(ExecutionContext& context);

int main(int argc, char** argv)
{
  lbcpp::initialize(argv[0]);
  ExecutionContextPtr context = defaultConsoleExecutionContext();
  declareProteinClasses(*context);

  int exitCode;

  if (argc == 1)
  {
    usage();
    exitCode = 0;
  }
  else if (argc == 2)
  {
    // load from serialization
    File parametersFile = File::getCurrentWorkingDirectory().getChildFile(argv[1]);
    ObjectPtr obj = Object::createFromFile(*context, parametersFile);
    
    if (obj && obj->getClass()->inheritsFrom(workUnitClass))
    {
      WorkUnitPtr workUnit = obj.staticCast<WorkUnit>();
      exitCode = context->run(workUnit) ? 0 : 1;
    }
  }
  else if (argc > 2)
  {
    // load program from string
    TypePtr type = context->getType(argv[1]);
    if (!type)
      exitCode = 1;
    else
    {
      ObjectPtr obj = context->createObject(type);
      if (obj && obj->getClass()->inheritsFrom(workUnitClass)) 
      {
        char** arguments = new char*[argc - 1];
        for (size_t i = 2; i < (size_t)argc; ++i)
          arguments[i - 1] = argv[i];
        arguments[0] = argv[0];
        exitCode = WorkUnit::main(*context, obj.staticCast<WorkUnit>(), argc - 1, arguments);
        delete[] arguments;
      }
    }
  }
  else
  {
    usage();
    exitCode = 1;
  }

  lbcpp::deinitialize();
  return exitCode;
}
