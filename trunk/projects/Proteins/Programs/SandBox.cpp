/*-----------------------------------------.---------------------------------.
| Filename: SandBox.cpp                    | Sand Box                        |
| Author  : Francis Maes                   |                                 |
| Started : 24/06/2010 11:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "WorkUnit/SandBoxWorkUnit.h"
using namespace lbcpp;

extern void declareProteinClasses(ExecutionContext& context);

int main(int argc, char** argv)
{
  lbcpp::initialize(argv[0]);

  int exitCode;
  {
    ExecutionContextPtr context = multiThreadedExecutionContext(8);
    context->appendCallback(consoleExecutionCallback());
    context->appendCallback(userInterfaceExecutionCallback());
    declareProteinClasses(*context);
    WorkUnitPtr workUnit(new SandBoxWorkUnit());
    exitCode = context->run(workUnit) ? 0 : 1;
  
    std::cout << "Waiting for GUI to close..." << std::endl;
    userInterfaceManager().waitUntilAllWindowsAreClosed();
    std::cout << "Tchao." << std::endl;
  }
  lbcpp::deinitialize();
  return exitCode;
}
