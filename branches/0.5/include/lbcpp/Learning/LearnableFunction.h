/*-----------------------------------------.---------------------------------.
| Filename: LearnableFunction.h            | Base classes for Learnable      |
| Author  : Francis Maes                   |  Functions                      |
| Started : 15/02/2011 18:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_LEARNABLE_FUNCTION_H_
# define LBCPP_LEARNING_LEARNABLE_FUNCTION_H_

# include <lbcpp/Core/Function.h>
# include "OnlineLearner.h"
# include "BatchLearner.h"

namespace lbcpp
{

class LearnerParameters : public Object
{
public:
  virtual BatchLearnerPtr createBatchLearner(ExecutionContext& context) const
    {return BatchLearnerPtr();}

  virtual OnlineLearnerPtr createOnlineLearner(ExecutionContext& context) const
    {return OnlineLearnerPtr();}
};

typedef ReferenceCountedObjectPtr<LearnerParameters> LearnerParametersPtr;

class SimpleLearnerParameters : public LearnerParameters
{
public:
  SimpleLearnerParameters(BatchLearnerPtr batchLearner, OnlineLearnerPtr onlineLearner)
    : batchLearner(batchLearner), onlineLearner(onlineLearner) {}

  virtual BatchLearnerPtr createBatchLearner(ExecutionContext& context) const
    {return batchLearner;}

  virtual OnlineLearnerPtr createOnlineLearner(ExecutionContext& context) const
    {return onlineLearner;}

protected:
  BatchLearnerPtr batchLearner;
  OnlineLearnerPtr onlineLearner;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_LEARNABLE_FUNCTION_H_
