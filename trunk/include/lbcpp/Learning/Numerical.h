/*-----------------------------------------.---------------------------------.
| Filename: Numerical.h                    | Numerical Learning              |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 19:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_H_
# define LBCPP_LEARNING_NUMERICAL_H_

# include "LearnableFunction.h"
# include "../Data/DoubleVector.h"

namespace lbcpp
{

class NumericalLearnableFunction : public LearnableFunction
{
public:
  const DoubleVectorPtr& getParameters() const
    {return parameters.staticCast<DoubleVector>();}

  DoubleVectorPtr& getParameters()
    {return *(DoubleVectorPtr* )&parameters;}

  // returns false if no supervision is available
  virtual bool computeAndAddGradient(const Variable* inputs, const Variable& output, double& exampleLossValue, DoubleVectorPtr& target, double weight) const = 0;
};

typedef ReferenceCountedObjectPtr<NumericalLearnableFunction> NumericalLearnableFunctionPtr;

extern NumericalLearnableFunctionPtr linearLearnableFunction();

extern OnlineLearnerPtr stochasticGDOnlineLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate = true);
extern OnlineLearnerPtr perEpisodeGDOnlineLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate = true);

extern FunctionPtr linearRegressor();

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_H_
