/*-----------------------------------------.---------------------------------.
| Filename: WorkUnit.h                     | Base class for Test Units       |
| Author  : Julien Becker                  |                                 |
| Started : 30/11/2010 14:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef OIL_EXECUTION_TEST_UNIT_H_
# define OIL_EXECUTION_TEST_UNIT_H_

# include "WorkUnit.h"

namespace lbcpp
{

class TestUnit : public WorkUnit
{
public:
  /** Numeric - Strict egality **/
  void checkEgality(ExecutionContext& context, int a, int b) const
    {if (a != b) context.errorCallback(JUCE_T("WorkUnit::checkEgality"), string(a) + JUCE_T(" is not equal to ") + string(b));}

  void checkEgality(ExecutionContext& context, size_t a, size_t b) const
    {if (a != b) context.errorCallback(JUCE_T("WorkUnit::checkEgality"), string((int)a) + JUCE_T(" is not equal to ") + string((int)b));}

  void checkEgality(ExecutionContext& context, double a, double b) const
    {if (a != b) context.errorCallback(JUCE_T("WorkUnit::checkEgality"), string(a) + JUCE_T(" is not equal to ") + string(b));}

  void checkEgality(ExecutionContext& context, bool a, bool b) const
    {if (a != b) context.errorCallback(JUCE_T("WorkUnit::checkEgality"), string(a) + JUCE_T(" is not equal to ") + string(b));}

  /** Numeric - Tolerant egality **/
  void checkIsCloseTo(ExecutionContext& context, double expected, double tolerance, double value) const
    {if (fabs(value - expected) > tolerance) context.errorCallback(JUCE_T("WorkUnit::checkIsCloseTo"), string(value) + JUCE_T(" is not enough close to ") + string(expected));}
  
  void checkIsCloseTo(ExecutionContext& context, int expected, double tolerance, int value) const
    {if (abs(value - expected) > tolerance) context.errorCallback(JUCE_T("WorkUnit::checkIsCloseTo"), string(value) + JUCE_T(" is not enough close to ") + string(expected));}
  
  /** Boolean **/
  void checkIsTrue(ExecutionContext& context, bool a) const
    {if (!a) context.errorCallback(JUCE_T("WorkUnit::checkIsTrue"), string(a) + JUCE_T(" is not true"));}
  
  void checkIsFalse(ExecutionContext& context, bool a) const
    {checkIsTrue(context, !a);}
};

}; /* namespace lbcpp */

#endif //!OIL_EXECUTION_TEST_UNIT_H_
