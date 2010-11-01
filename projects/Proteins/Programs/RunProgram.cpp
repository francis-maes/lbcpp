
#include "Programs/SurfBox.h"

using namespace lbcpp;

extern void declareProteinClasses();

class MuteMessageCallback : public MessageCallback
{
  virtual void errorMessage(const String& where, const String& what) {}
  virtual void warningMessage(const String& where, const String& what) {}
  virtual void infoMessage(const String& where, const String& what) {}
};

void usage()
{
  std::cout << "Usage ..." << std::endl;
}

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
    MuteMessageCallback muteCallback;
    ObjectPtr obj = Object::createFromFile(parametersFile, muteCallback);
    
    if (obj && obj->getClass()->canBeCastedTo(Type::get("Program", callback)))
    {
      ProgramPtr program = obj.staticCast<Program>();
      int exitCode = program->run(callback);
      lbcpp::deinitialize();
      return exitCode;
    }
  }

  // load program from string
  ObjectPtr obj = Object::create(Type::get(argv[1], callback));
  if (obj && obj->getClass()->canBeCastedTo(Type::get("Program", callback)))
  {
    ProgramPtr program = obj.staticCast<Program>();
    int exitCode = program->run(callback);
    lbcpp::deinitialize();
    return exitCode;
  }
  
  usage();
  return -1;
}
