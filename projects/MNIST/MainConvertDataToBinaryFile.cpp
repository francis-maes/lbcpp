/*-----------------------------------------.---------------------------------.
| Filename: MainLearner.cpp                | Learner                         |
| Author  : Julien Becker                  |                                 |
| Started : 10/11/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Program/BinarizerProgram.h"

using namespace lbcpp;

extern void declareMNISTClasses(ExecutionContext& context);

int main(int argc, char* argv[])
{
  lbcpp::initialize();
  ExecutionContextPtr context = defaultConsoleExecutionContext();
  declareMNISTClasses(*context);
  return WorkUnit::main(new BinarizerProgram(), argc, argv);
}
