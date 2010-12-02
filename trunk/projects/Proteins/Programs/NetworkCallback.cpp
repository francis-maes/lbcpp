/*-----------------------------------------.---------------------------------.
| Filename: NetworkCallback.cpp            | Network Callback                |
| Author  : Julien Becker                  |                                 |
| Started : 01/12/2010 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "NetworkCallback.h"

using namespace lbcpp;

/** BufferedMessageCallback **/
Variable BufferedNetworkCallback::receiveVariable(bool blocking)
{
  if (!blocking)
    return lockedPop();

  Variable res;
  do
  {
    res = lockedPop();
    if (!res.isNil() || !connected)
      return res;
    juce::Thread::sleep(1000);
  } while (true);
}

void BufferedNetworkCallback::variableReceived(const Variable& variable)
{
  ScopedLock _(lock);
  variables.push_back(variable);
}

Variable BufferedNetworkCallback::lockedPop()
{
  ScopedLock _(lock);
  Variable res;
  if (variables.size())
  {
    res = variables.front();
    variables.pop_front();
  }
  return res;
}

void BufferedNetworkCallback::disconnected()
  {connected = false;}
