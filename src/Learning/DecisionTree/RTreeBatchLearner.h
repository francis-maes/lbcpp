/*-----------------------------------------.---------------------------------.
| Filename: RTreeLearner.h                 | Batch Learner of ExtraTrees     |
| Author  : Julien Becker                  | implemented by Pierre Geurts    |
| Started : 16/02/2011 13:35               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DECISION_TREE_R_TREE_LEARNER_H_
# define LBCPP_DECISION_TREE_R_TREE_LEARNER_H_

# include <lbcpp/Learning/BatchLearner.h>
# include "RTreeFunction.h"

namespace lbcpp
{

class RTreeBatchLearner : public BatchLearner
{
public:
  RTreeBatchLearner()
    {numInputs = 2;}
  
  virtual bool train(ExecutionContext& context, const FunctionPtr& function, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DECISION_TREE_R_TREE_LEARNER_H_
