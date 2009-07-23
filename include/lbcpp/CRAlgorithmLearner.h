/*
** $PROJECT_PRESENTATION_AND_CONTACT_INFOS$
**
** Copyright (C) 2009 Francis MAES
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
