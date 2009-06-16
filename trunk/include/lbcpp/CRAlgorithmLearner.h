/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmLearner.h           | CR-algorithm Learners base class|
| Author  : Francis Maes                   |                                 |
| Started : 11/06/2009 17:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   CRAlgorithmLearner.h
**@author Francis MAES
**@date   Fri Jun 12 16:57:27 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_CRALGORITHM_LEARNER_H_
# define LBCPP_CRALGORITHM_LEARNER_H_

# include "LearningMachine.h"
# include "CRAlgorithm.h"
# include "IterationFunction.h"
# include "StoppingCriterion.h"

namespace lbcpp
{

/*!
** @class CRAlgorithmLearner
** @brief
*/
class CRAlgorithmLearner : public LearningMachine
{
public:
  /*!
  **
  **
  **
  ** @return
  */
  virtual PolicyPtr getPolicy() const = 0;
};

enum ExplorationType
{
  optimal,
  optimalEpsilonGreedy,
  optimalGibbsGreedy,
  optimalToPredicted,
  predicted,
  predictedEpsilonGreedy,
  predictedGibbsGreedy,
  predictedProbabilistic, // only for policies represented as probabilistic classifier 
  random,
};
  
/*!
**
**
** @param numIterations
**
** @return
*/
extern CRAlgorithmLearnerPtr searnLearner(RankerPtr ranker = RankerPtr(),
    ActionValueFunctionPtr optimalActionValues = ActionValueFunctionPtr(),
    double beta = 0.1,
    StoppingCriterionPtr stoppingCriterion = StoppingCriterionPtr());

extern CRAlgorithmLearnerPtr sarsaLearner(RegressorPtr regressor = RegressorPtr(),
      double discount = 0.95,
      ExplorationType exploration = predictedEpsilonGreedy,
      IterationFunctionPtr explorationParameter = constantIterationFunction(0.1),
      StoppingCriterionPtr stoppingCriterion = maxIterationsStoppingCriterion(100));

}; /* namespace lbcpp */

#endif // !LBCPP_CRALGORITHM_LEARNER_H_
