/*-----------------------------------------.---------------------------------.
| Filename: MainPredictor.cpp              | Predictor                       |
| Author  : Julien Becker                  |                                 |
| Started : 10/11/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Program/PredictorProgram.h"

using namespace lbcpp;

extern void declareMNISTClasses(ExecutionContext& context);

int main(int argc, char* argv[])
{
  lbcpp::initialize(argv[0]);
  ExecutionContextPtr context = defaultConsoleExecutionContext(false);
  context->appendCallback(consoleExecutionCallback());
  declareMNISTClasses(*context);
  return WorkUnit::main(*context, new PredictorProgram(), argc, argv);
}
