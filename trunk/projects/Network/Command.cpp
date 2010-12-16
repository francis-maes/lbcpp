/*-----------------------------------------.---------------------------------.
| Filename: Command.cpp                    | Network Command                 |
| Author  : Julien Becker                  |                                 |
| Started : 02/12/2010 18:52               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "Command.h"

using namespace lbcpp;

/** Command **/
bool Command::runCommand(ExecutionContext& context, NetworkClientPtr client)
{
  jassert(client);
  this->client = client;
  return run(context);
}

namespace lbcpp
{



}; /* namespace lbcpp */
