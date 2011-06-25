/*-----------------------------------------.---------------------------------.
| Filename: LogicalOrStoppingCriterion.h   | Logicial Or Stopping            |
| Author  : Francis Maes                   |  Criterion                      |
| Started : 25/08/2010 22:10               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_STOPPING_CRITERION_LOGICAL_OR_H_
# define LBCPP_FUNCTION_STOPPING_CRITERION_LOGICAL_OR_H_

# include <lbcpp/Function/StoppingCriterion.h>

namespace lbcpp
{

class LogicalOrStoppingCriterion : public StoppingCriterion
{
public:
  LogicalOrStoppingCriterion(StoppingCriterionPtr criterion1, StoppingCriterionPtr criterion2)
    : criterion1(criterion1), criterion2(criterion2) {}
  LogicalOrStoppingCriterion() {}

  virtual void reset()
    {criterion1->reset(); criterion2->reset();}

  virtual bool shouldStop(double value)
  {
    bool t1 = criterion1->shouldStop(value);
    bool t2 = criterion2->shouldStop(value);
    return t1 || t2;
  }

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    ReferenceCountedObjectPtr<LogicalOrStoppingCriterion> t = target.staticCast<LogicalOrStoppingCriterion>();
    t->criterion1 = criterion1->cloneAndCast<StoppingCriterion>(context);
    t->criterion2 = criterion2->cloneAndCast<StoppingCriterion>(context);
  }

  lbcpp_UseDebuggingNewOperator

private:
  friend class LogicalOrStoppingCriterionClass;

  StoppingCriterionPtr criterion1;
  StoppingCriterionPtr criterion2;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_STOPPING_CRITERION_LOGICAL_OR_H_