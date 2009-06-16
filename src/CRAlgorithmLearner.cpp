/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmLearner.cpp         | CR-algorithm Learners base class|
| Author  : Francis Maes                   |                                 |
| Started : 11/06/2009 17:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/CRAlgorithmLearner.h>
#include "CRAlgorithmLearner/SearnLearner.h"
#include "CRAlgorithmLearner/SarsaLearner.h"
using namespace lbcpp;

CRAlgorithmLearnerPtr lbcpp::searnLearner(RankerPtr ranker, ActionValueFunctionPtr optimalActionValues, double beta, StoppingCriterionPtr stoppingCriterion)
{
  return new SearnLearner(ranker, optimalActionValues, beta, stoppingCriterion);
}

CRAlgorithmLearnerPtr lbcpp::sarsaLearner(RegressorPtr regressor, double discount, ExplorationType exploration, IterationFunctionPtr explorationParameter, StoppingCriterionPtr stoppingCriterion)
{
  return new SarsaLearner(regressor, discount, exploration, explorationParameter, stoppingCriterion);
}

/*
** Serializable classes declaration
*/
void declareCRAlgorithmLearners()
{
  LBCPP_DECLARE_CLASS(SearnLearner);
  LBCPP_DECLARE_CLASS(SarsaLearner);
}
