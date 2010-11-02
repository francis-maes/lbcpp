
#include <lbcpp/lbcpp.h>
#include "Programs/Program.h"

using namespace lbcpp;

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

extern void declareProteinClasses();
extern void declareProgramClasses();

namespace lbcpp
{
  extern ClassPtr programClass;
}

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
  
  MessageCallback& callback = MessageCallback::getInstance();
  if (argc == 2)
  {
    // load from serialization
    File parametersFile = File::getCurrentWorkingDirectory().getChildFile(argv[1]);
    MuteMessageCallback muteCallback;
    ObjectPtr obj = Object::createFromFile(parametersFile, muteCallback);
    
    if (obj && obj->getClass()->inheritsFrom(programClass))
    {
      ProgramPtr program = obj.staticCast<Program>();
      int exitCode = program->runProgram(callback);
      lbcpp::deinitialize();
      return exitCode;
    }
  }

  // load program from string
  ObjectPtr obj = Object::create(Type::get(argv[1], callback));
  if (obj && obj->getClass()->inheritsFrom(Type::get("Program", callback)))
  {
    std::vector<String> arguments;
    for (size_t i = 2; i < (size_t)argc; ++i)
      arguments.push_back(argv[i]);

    ProgramPtr program = obj.staticCast<Program>();
    if (!program->parseArguments(arguments, callback))
      return -1;
    int exitCode = program->runProgram(callback);
    lbcpp::deinitialize();
    return exitCode;
  }
  
  usage();
  return -1;
}
