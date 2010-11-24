
#include <lbcpp/lbcpp.h>

using namespace lbcpp;

void usage()
{
  std::cout << "Usage ..." << std::endl;
}

extern void declareProteinClasses();
extern void declareProgramClasses();

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();
  declareProgramClasses();

  if (argc == 1)
  {
    usage();
    return 0;
  }
  
  ExecutionContextPtr context = singleThreadedExecutionContext();
  context->appendCallback(consoleExecutionCallback());
  
  if (argc == 2)
  {
    // load from serialization
    File parametersFile = File::getCurrentWorkingDirectory().getChildFile(argv[1]);
    //MuteMessageCallback muteCallback; // FIXME: mute is not mute anymore
    ObjectPtr obj = Object::createFromFile(parametersFile);//, muteCallback);
    
    if (obj && obj->getClass()->inheritsFrom(workUnitClass))
    {
      WorkUnitPtr workUnit = obj.staticCast<WorkUnit>();
      bool ok = context->run(workUnit);
      lbcpp::deinitialize();
      return ok ? 0 : 1;
    }
  }

  // load program from string
  ObjectPtr obj = Object::create(Type::get(argv[1]));
  if (obj && obj->getClass()->inheritsFrom(workUnitClass)) 
  {
    char** arguments = new char*[argc - 1];
    for (size_t i = 2; i < (size_t)argc; ++i)
      arguments[i - 1] = argv[i];
    arguments[0] = argv[0];

    int exitCode = WorkUnit::main(obj.staticCast<WorkUnit>(), argc - 1, arguments);
    delete[] arguments;
    lbcpp::deinitialize();
    return exitCode;
  }
  
  usage();
  return -1;
}
