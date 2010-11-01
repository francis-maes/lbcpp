
#include "Programs/SurfBox.h"

using namespace lbcpp;

void usage()
{
  std::cout << "Usage ..." << std::endl;
}

extern void declareProteinClasses();

int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();

  if (argc == 1)
  {
    usage();
    return 0;
  }
  
  MessageCallback& callback = MessageCallback::getInstance();
  if (argc == 2)
  {
    // load from serialization
    File parametersFile = File::getCurrentWorkingDirectory().getChildFile(argv[1]);
    ObjectPtr obj = Object::createFromFile(parametersFile, callback);
    if (obj && obj->getClass()->canBeCastedTo(Type::get("Program", callback)))
    {
      ProgramPtr program = obj.staticCast<Program>();
      return program->run(callback);
    }
    
    // load program from string
    obj = Object::create(Type::get(argv[1], callback));
    if (obj && obj->getClass()->canBeCastedTo(Type::get("Program", callback)))
    {
      ProgramPtr program = obj.staticCast<Program>();
      if (program->getNumVariables())
      { // TODO open a shell and ask parameters
        callback.errorMessage(T("main"), T("Not enough parameters"));
        usage();
        return -1;
      }
      return program->run(callback);
    }
  }
  
  callback.errorMessage(T("main"), T("Not yet implemented"));
  return 0;

  lbcpp::deinitialize();
}