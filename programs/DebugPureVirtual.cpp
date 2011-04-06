/*-----------------------------------------.---------------------------------.
| Filename: RunWorkUnit.cpp                | A program to launch work units  |
| Author  : Francis Maes                   |                                 |
| Started : 16/12/2010 12:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/library.h>

using namespace lbcpp;

int main(int argc, char** argv)
{
  lbcpp::initialize(argv[0]);
  {
  File projectDirectory = File::getCurrentWorkingDirectory();
  ExecutionContextPtr context = singleThreadedExecutionContext(projectDirectory);
  setDefaultExecutionContext(context);
  }
  lbcpp::deinitialize();
  return 0;
}
