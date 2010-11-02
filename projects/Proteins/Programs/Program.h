
#ifndef LBCPP_PROTEINS_PROGRAMS_PROGRAM_H_
# define LBCPP_PROTEINS_PROGRAMS_PROGRAM_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class Program : public Object
{
public:
  virtual ~Program() {}

  virtual int runProgram(MessageCallback& callback = MessageCallback::getInstance())
  {
    callback.errorMessage(T("Program::run"), T("Program not yet implemented !"));
    return 0;
  }

  virtual String toString() const
    {return T("No description available !");}

  /* Generic */
  virtual String toShortString() const
    {return getClassName();}

  virtual String getName() const
    {return getClassName();}

  virtual bool parseArguments(const std::vector<String>& arguments, MessageCallback& callback = MessageCallback::getInstance());

  virtual String getUsage() const;
};

typedef ReferenceCountedObjectPtr<Program> ProgramPtr;

};

#endif // !LBCPP_PROTEINS_PROGRAMS_PROGRAM_H_