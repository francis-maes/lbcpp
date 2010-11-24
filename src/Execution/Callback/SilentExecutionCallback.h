/*-----------------------------------------.---------------------------------.
| Filename: SilentExecutionCallback.h      | Silent Execution Callback       |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 17:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CALLBACK_SILENT_H_
# define LBCPP_EXECUTION_CALLBACK_SILENT_H_

# include <lbcpp/Execution/ExecutionCallback.h>

namespace lbcpp
{

class SilentExecutionCallback : public ExecutionCallback
{
public:
  virtual void informationCallback(const String& where, const String& what) {}
  virtual void warningCallback(const String& where, const String& what) {}
  virtual void errorCallback(const String& where, const String& what) {}

  virtual void statusCallback(const String& status) {}
  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit) {}
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CALLBACK_SILENT_H_
