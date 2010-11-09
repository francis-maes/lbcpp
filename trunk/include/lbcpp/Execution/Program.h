/*-----------------------------------------.---------------------------------.
 | Filename: Program.h                      | Program                         |
 | Author  : Julien Becker                  |                                 |
 | Started : 09/11/2010 12:41               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/


#ifndef LBCPP_EXECUTION_PROGRAM_H_
# define LBCPP_EXECUTION_PROGRAM_H_

# include <lbcpp/lbcpp.h>

namespace lbcpp
{

class Program : public Object
{
public:
  virtual ~Program() {}
  
  int main(int argc, char* argv[]);

  virtual int runProgram(MessageCallback& callback = MessageCallback::getInstance())
  {
    callback.errorMessage(T("Program::run"), T("Program not yet implemented !"));
    return 0;
  }

  virtual String toString() const
    {return T("No description available !");}

  /* Object */
  virtual String toShortString() const
    {return getClassName();}

  virtual String getName() const
    {return getClassName();}

protected:
  bool parseArguments(const std::vector<String>& arguments, MessageCallback& callback = MessageCallback::getInstance());

  String getUsage() const;

private:
  bool helpRequired(const std::vector<String>& arguments) const;
};

typedef ReferenceCountedObjectPtr<Program> ProgramPtr;

};

#endif // !LBCPP_EXECUTION_PROGRAM_H_
