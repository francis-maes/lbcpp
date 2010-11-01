
#ifndef LBCPP_PROTEINS_PROGRAMS_PROGRAM_H_
# define LBCPP_PROTEINS_PROGRAMS_PROGRAM_H_

# include <lbcpp/lbcpp.h>

using namespace lbcpp;

class Program : public NameableObject
{
public:
  Program(const String& name = T("Unnamed Program"))
    : NameableObject(name) {}

  virtual int run(MessageCallback& callback = MessageCallback::getInstance())
  {
    callback.errorMessage(T("Program::run"), T("Program not yet implemented !"));
    return 0;
  }
  
  virtual String description()
  {
    return T("No description available !");
  }
  
  virtual ~Program() {}
};

typedef ReferenceCountedObjectPtr<Program> ProgramPtr;

#endif // !LBCPP_PROTEINS_PROGRAMS_PROGRAM_H_