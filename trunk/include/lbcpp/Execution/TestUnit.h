/*-----------------------------------------.---------------------------------.
| Filename: WorkUnit.h                     | Base class for Test Units       |
| Author  : Julien Becker                  |                                 |
| Started : 30/11/2010 14:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_TEST_UNIT_H_
# define LBCPP_EXECUTION_TEST_UNIT_H_

# include "WorkUnit.h"

namespace lbcpp
{

class TestUnit : public WorkUnit
{
public:
  /** Numeric - Strict egality **/
  void checkEgality(ExecutionContext& context, int a, int b) const
    {if (a != b) context.errorCallback(T("WorkUnit::checkEgality"), String(a) + T(" is not equal to ") + String(b));}

  void checkEgality(ExecutionContext& context, double a, double b) const
    {if (a != b) context.errorCallback(T("WorkUnit::checkEgality"), String(a) + T(" is not equal to ") + String(b));}

  void checkEgality(ExecutionContext& context, bool a, bool b) const
    {if (a != b) context.errorCallback(T("WorkUnit::checkEgality"), String(a) + T(" is not equal to ") + String(b));}

  /** Numeric - Tolerant egality **/
  void checkIsCloseTo(ExecutionContext& context, double expected, double tolerance, double value) const
    {if (abs(value - expected) > tolerance) context.errorCallback(T("WorkUnit::checkIsCloseTo"), String(value) + T(" is not enough close to ") + String(expected));}
  
  void checkIsCloseTo(ExecutionContext& context, int expected, double tolerance, int value) const
    {if (abs(value - expected) > tolerance) context.errorCallback(T("WorkUnit::checkIsCloseTo"), String(value) + T(" is not enough close to ") + String(expected));}
  
  /** Boolean **/
  void checkIsTrue(ExecutionContext& context, bool a) const
    {if (!a) context.errorCallback(T("WorkUnit::checkIsTrue"), String(a) + T(" is not true"));}
  
  void checkIsFalse(ExecutionContext& context, bool a) const
    {checkIsTrue(context, !a);}
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_TEST_UNIT_H_
