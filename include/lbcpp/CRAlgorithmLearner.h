/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmLearner.h           | CR-algorithm Learners base class|
| Author  : Francis Maes                   |                                 |
| Started : 11/06/2009 17:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_CRALGORITHM_LEARNER_H_
# define LBCPP_CRALGORITHM_LEARNER_H_

# include "LearningMachine.h"
# include "CRAlgorithm.h"

namespace lbcpp
{

class CRAlgorithmLearner : public LearningMachine
{
public:
  virtual PolicyPtr getPolicy() const = 0;
};

extern CRAlgorithmLearnerPtr searnLearner(RankerPtr ranker = RankerPtr(), ActionValueFunctionPtr optimalActionValues = ActionValueFunctionPtr(), double beta = 0.1, size_t numIterations = 10);

}; /* namespace lbcpp */

#endif // !LBCPP_CRALGORITHM_LEARNER_H_
