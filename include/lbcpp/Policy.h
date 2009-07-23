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
| Filename: Policy.h                       | CR-algorithm policy             |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 17:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   Policy.h
**@author Francis MAES
**@date   Fri Jun 12 19:20:10 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_POLICY_H_
# define LBCPP_POLICY_H_

# include "ObjectPredeclarations.h"
# include "RandomVariable.h" // only for IterationFuction

namespace lbcpp
{

/*!
** @class Policy
** @brief
*/
class Policy : public Object
{
public:
  /*!
  **
  **
  ** @param crAlgorithm
  **
  ** @return
  */
  bool run(CRAlgorithmPtr crAlgorithm, PolicyStatisticsPtr statistics = PolicyStatisticsPtr());

  /*!
  **
  **
  ** @param crAlgorithms
  ** @param progress
  **
  ** @return
  */
  bool run(ObjectStreamPtr crAlgorithms, PolicyStatisticsPtr statistics = PolicyStatisticsPtr(), ProgressCallbackPtr progress = ProgressCallbackPtr());

  /*!
  **
  **
  ** @param crAlgorithms
  ** @param progress
  **
  ** @return
  */
  bool run(ObjectContainerPtr crAlgorithms, PolicyStatisticsPtr statistics = PolicyStatisticsPtr(), ProgressCallbackPtr progress = ProgressCallbackPtr());

  PolicyStatisticsPtr computeStatistics(ObjectStreamPtr crAlgorithms, ProgressCallbackPtr progress = ProgressCallbackPtr())
    {PolicyStatisticsPtr res = new PolicyStatistics(); run(crAlgorithms, res, progress); return res;}

  PolicyStatisticsPtr computeStatistics(ObjectContainerPtr crAlgorithms, ProgressCallbackPtr progress = ProgressCallbackPtr())
    {PolicyStatisticsPtr res = new PolicyStatistics(); run(crAlgorithms, res, progress); return res;}

  /*!
  **
  **
  ** @param verbosity
  ** @param ostr
  **
  ** @return
  */
  PolicyPtr verbose(size_t verbosity, std::ostream& ostr = std::cout) const;

public:
  /*!
  **
  **
  ** @param crAlgorithm
  */
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm) {}

  /*!
  **
  **
  ** @param choose
  **
  ** @return
  */
  virtual VariablePtr policyChoose(ChoosePtr choose) = 0;

  /*!
  **
  **
  ** @param reward
  */
  virtual void policyReward(double reward) {}

  /*!
  **
  **
  */
  virtual void policyLeave() {}

  /*!
  ** deprecated
  **
  **
  ** @return
  */
  virtual size_t getNumResults() const {return 0;}

  /*!
  ** deprecated
  **
  ** @param i
  **
  ** @return
  */
  virtual ObjectPtr getResult(size_t i) const
    {assert(false); return ObjectPtr();}

  /*!
  **
  **
  ** @param name
  **
  ** @return
  */
  virtual ObjectPtr getResultWithName(const std::string& name) const;
};

/*!
**
**
**
** @return
*/
extern PolicyPtr randomPolicy();

/*!
**
**
** @param actionValues
**
** @return
*/
extern PolicyPtr greedyPolicy(ActionValueFunctionPtr actionValues);

/*!
**
**
** @param actionValue
** @param temperature
**
** @return
*/
extern PolicyPtr gibbsGreedyPolicy(ActionValueFunctionPtr actionValue, IterationFunctionPtr temperature);

/*!
**
**
** @param actionProbabilities
**
** @return
*/
extern PolicyPtr stochasticPolicy(ActionValueFunctionPtr actionProbabilities);

/*!
**
**
** @param basePolicy
** @param epsilon
**
** @return
*/
extern PolicyPtr epsilonGreedyPolicy(PolicyPtr basePolicy, IterationFunctionPtr epsilon);

// mixtureCoefficient = Probability of selecting policy2
/*!
**
**
** @param policy1
** @param policy2
** @param mixtureCoefficient
**
** @return
*/
extern PolicyPtr mixturePolicy(PolicyPtr policy1, PolicyPtr policy2, double mixtureCoefficient = 0.5);

extern PolicyPtr computeStatisticsPolicy(PolicyPtr policy, PolicyStatisticsPtr statistics);

/*!
**
**
** @param explorationPolicy
** @param regressor
** @param discount
**
** @return
*/
extern PolicyPtr qlearningPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);

/*!
**
**
** @param explorationPolicy
** @param regressor
** @param discount
**
** @return
*/
extern PolicyPtr sarsaZeroPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);

/*!
**
**
** @param explorationPolicy
** @param regressor
** @param discount
**
** @return
*/
extern PolicyPtr monteCarloControlPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);

/*!
**
**
** @param explorationPolicy
** @param classifier
** @param supervisor
**
** @return
*/
extern PolicyPtr classificationExamplesCreatorPolicy(PolicyPtr explorationPolicy,
                        ClassifierPtr classifier,
                        ActionValueFunctionPtr supervisor = ActionValueFunctionPtr());

/*!
**
**
** @param explorationPolicy
** @param ranker
** @param supervisor
**
** @return
*/
extern PolicyPtr rankingExamplesCreatorPolicy(PolicyPtr explorationPolicy,
                        RankerPtr ranker,
                        ActionValueFunctionPtr supervisor = ActionValueFunctionPtr());

/*!
**
**
** @param classifier
** @param beta
** @param exploration
**
** @return
*/
extern PolicyPtr gpomdpPolicy(GeneralizedClassifierPtr classifier, double beta, double exploration = 1.0);

/*!
**
**
** @param classifier
** @param beta
** @param explorationPolicy
**
** @return
*/
extern PolicyPtr gpomdpPolicy(GeneralizedClassifierPtr classifier, double beta, PolicyPtr explorationPolicy);

/*!
** @class DecoratorPolicy
** @brief
*/
class DecoratorPolicy : public Policy
{
public:
  /*!
  **
  **
  ** @param decorated
  **
  ** @return
  */
  DecoratorPolicy(PolicyPtr decorated = PolicyPtr())
    : decorated(decorated) {}

  /*!
  **
  **
  ** @param crAlgorithm
  */
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
    {decorated->policyEnter(crAlgorithm);}

  /*!
  **
  **
  ** @param choose
  **
  ** @return
  */
  virtual VariablePtr policyChoose(ChoosePtr choose)
    {return decorated->policyChoose(choose);}

  /*!
  **
  **
  ** @param reward
  */
  virtual void policyReward(double reward)
    {decorated->policyReward(reward);}

  /*!
  **
  **
  */
  virtual void policyLeave()
    {decorated->policyLeave();}

  /*!
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const
    {write(ostr, decorated);}

  /*!
  **
  **
  ** @param istr
  **
  ** @return
  */
  virtual bool load(std::istream& istr)
    {return read(istr, decorated);}

protected:
  PolicyPtr decorated;          /*!< */
};

class EpisodicPolicy : public Policy
{
public:
  EpisodicPolicy();

  virtual VariablePtr policyStart(ChoosePtr choose) = 0;
  virtual VariablePtr policyStep(double reward, ChoosePtr choose) = 0;
  virtual void policyEnd(double reward) = 0;

  virtual void policyEnter(CRAlgorithmPtr crAlgorithm);
  virtual VariablePtr policyChoose(ChoosePtr choose);
  virtual void policyReward(double reward);
  virtual void policyLeave();

protected:
  size_t inclusionLevel;
  size_t stepNumber;
  double currentReward;
};

class EpisodicDecoratorPolicy : public EpisodicPolicy
{
public:
  EpisodicDecoratorPolicy(PolicyPtr decorated = PolicyPtr())
    : decorated(decorated) {}

  virtual VariablePtr policyStart(ChoosePtr choose)
  {
    decorated->policyEnter(choose->getCRAlgorithm());
    return decorated->policyChoose(choose);
  }

  virtual VariablePtr policyStep(double reward, ChoosePtr choose)
  {
    if (reward)
      decorated->policyReward(reward);
    return decorated->policyChoose(choose);
  }

  virtual void policyEnd(double reward)
  {
    if (reward)
      decorated->policyReward(reward);
    decorated->policyLeave();
  }

  /*!
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const
    {write(ostr, decorated);}

  /*!
  **
  **
  ** @param istr
  **
  ** @return
  */
  virtual bool load(std::istream& istr)
    {return read(istr, decorated);}

protected:
  PolicyPtr decorated;
};

}; /* namespace lbcpp */

#endif // !LBCPP_POLICY_H_
