#include <lbcpp/lbcpp.h>
#include "ManagerServer.h"

using namespace lbcpp;

int main(int argc, char** argv)
{
  lbcpp::initialize(argv[0]);
  ExecutionContextPtr context = singleThreadedExecutionContext();
  context->appendCallback(consoleExecutionCallback());

  int exitCode;
  {
    exitCode = WorkUnit::main(*context, new ManagerServer(), argc, argv);
  }

  lbcpp::deinitialize();
  return exitCode;
}
