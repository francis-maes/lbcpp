/*-----------------------------------------.---------------------------------.
| Filename: MainLearner.cpp                | Learner                         |
| Author  : Julien Becker                  |                                 |
| Started : 10/11/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Program/LearnerProgram.h"
using namespace lbcpp;

extern void declareMNISTClasses(ExecutionContext& context);

int main(int argc, char* argv[])
{
  lbcpp::initialize(argv[0]);
  int exitCode;
  {
    ExecutionContextPtr context = defaultConsoleExecutionContext();
    context->appendCallback(userInterfaceExecutionCallback());
    declareMNISTClasses(*context);
    exitCode = WorkUnit::main(*context, new LearnerProgram(), argc, argv);
    std::cout << "Waiting for GUI to close..." << std::endl;
    userInterfaceManager().waitUntilAllWindowsAreClosed();
    std::cout << "Tchao." << std::endl;
  }
  lbcpp:deinitialize();
  return exitCode;
}
