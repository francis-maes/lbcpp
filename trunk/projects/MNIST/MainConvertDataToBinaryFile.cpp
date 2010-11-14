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

extern void declareMNISTClasses();

int main(int argc, char* argv[])
{
  lbcpp::initialize();
  declareMNISTClasses();
  return ProgramPtr(new BinarizerProgram())->main(argc, argv);
}
