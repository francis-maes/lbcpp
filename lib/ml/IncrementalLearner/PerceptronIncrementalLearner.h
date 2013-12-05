/*-----------------------------------------.---------------------------------.
| Filename: PerceptronIncrementalLearner.h | Perceptron                      |
| Author  : Denny Verbeeck                 | Incremental Linear Perceptron   |
| Started : 05/12/2013 11:17               | Learner                         |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_PERCEPTRON_INCREMENTAL_LEARNER_H_
# define ML_PERCEPTRON_INCREMENTAL_LEARNER_H_

# include <ml/IncrementalLearner.h>
# include <ml/Expression.h>

namespace lbcpp
{

class PerceptronIncrementalLearner : public IncrementalLearner
{
public:
  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const
  {
  }

  virtual void addTrainingSample(ExecutionContext &context, const std::vector< ObjectPtr > &sample, ExpressionPtr expression) const
  {
  }

};

} /* namespace lbcpp */

#endif //!ML_PERCEPTRON_INCREMENTAL_LEARNER_H_
