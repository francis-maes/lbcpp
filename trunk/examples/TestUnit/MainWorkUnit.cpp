#include <lbcpp/lbcpp.h>
#include <lbcpp/Execution/ExecutionContext.h>
#include <lbcpp/Execution/TestUnit.h>
#include "ExtraTreeTestUnit.h"
using namespace lbcpp;

namespace lbcpp { extern LibraryPtr testUnitLibrary;};

int main(int argc, char* argv[])
{
  lbcpp::initialize(argv[0]);
  {
    ExecutionContextPtr context = multiThreadedExecutionContext(8); //defaultConsoleExecutionContext(false);
    lbcpp::setDefaultExecutionContext(context);
    context->appendCallback(consoleExecutionCallback());
    context->appendCallback(userInterfaceExecutionCallback());
    lbcpp::importLibrary(testUnitLibrary);
    
    std::vector<TestUnitPtr> units;
    units.push_back(new ExtraTreeTestUnit());
    
    for (size_t i = 0; i < units.size(); ++i)
      WorkUnit::main(*context, units[i], argc, argv);

    userInterfaceManager().waitUntilAllWindowsAreClosed();
    lbcpp::setDefaultExecutionContext(ExecutionContextPtr());
  }
  lbcpp::deinitialize();
  return 0;
}
