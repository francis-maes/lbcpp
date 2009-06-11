/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithmLearner.cpp         | CR-algorithm Learners base class|
| Author  : Francis Maes                   |                                 |
| Started : 11/06/2009 17:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/CRAlgorithmLearner.h>
#include "CRAlgorithmLearner/SearnLearner.h"
using namespace lbcpp;

CRAlgorithmLearnerPtr lbcpp::searnLearner(RankerPtr ranker, ActionValueFunctionPtr optimalActionValues, double beta, size_t numIterations)
{
  return new SearnLearner(ranker, optimalActionValues, beta, numIterations);
}
