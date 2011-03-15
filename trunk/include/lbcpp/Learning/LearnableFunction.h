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

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_LEARNABLE_FUNCTION_H_
