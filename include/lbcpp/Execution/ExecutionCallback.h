/*-----------------------------------------.---------------------------------.
| Filename: ExecutionCallback.h            | Execution Callback Base Class   |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 17:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CALLBACK_H_
# define LBCPP_EXECUTION_CALLBACK_H_

# include "../Data/Object.h"

namespace lbcpp
{

class ExecutionCallback : public Object
{
public:
  /*
  ** Informations
  */
  virtual void informationCallback(const String& where, const String& what) = 0;

  void informationCallback(const String& what)
    {informationCallback(String::empty, what);}

  /*
  ** Warnings and Errors
  */
  virtual void warningCallback(const String& where, const String& what) = 0;
  virtual void errorCallback(const String& where, const String& what) = 0;

  /*
  ** Status and Progression
  */
  virtual void statusCallback(const String& status) = 0;
  virtual void progressCallback(double progression, double progressionTotal, const String& progressionUnit) = 0;

  void progressCallback(double normalizedProgression)
    {progressCallback(normalizedProgression * 100.0, 100.0, T("%"));}
};

typedef ReferenceCountedObjectPtr<ExecutionCallback> ExecutionCallbackPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CALLBACK_H_
