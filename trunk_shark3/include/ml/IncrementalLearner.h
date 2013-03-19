/*-----------------------------------------.---------------------------------.
| Filename: IncrementalLearner.h           | Incremental Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 14/03/2013 14:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_INCREMENTAL_LEARNER_H_
# define ML_INCREMENTAL_LEARNER_H_

# include "Expression.h"

namespace lbcpp
{
  
class IncrementalLearner : public Object
{
public:
  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const = 0;
  virtual void addTrainingSample(ExecutionContext& context, const std::vector<ObjectPtr>& sample, ExpressionPtr expression) const = 0;
};

typedef ReferenceCountedObjectPtr<IncrementalLearner> IncrementalLearnerPtr;

}; /* namespace lbcpp */

#endif // !ML_INCREMENTAL_LEARNER_H_
