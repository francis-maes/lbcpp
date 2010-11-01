

#include "Programs/Program.h"

using namespace lbcpp;

class HelloWorldProgram : public Program
{
public:
  HelloWorldProgram() : Program(T("Hello World"))
  {
    myInt = 4;
  }
  
  virtual int run(MessageCallback& callback)
  {
    std::cout << "Hello World : " << myInt << std::endl;
    return 0;
  }
  
  virtual String description()
  {
    return T("It's just a test program ! Don't worry about it !");
  }
  
  int myInt;
};

