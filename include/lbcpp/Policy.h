/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
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
**@brief  Policy class declaration.
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
** @brief A Policy describe the way choices are made.
*/
class Policy : public Object
{
public:
  /**
  ** Runs the current policy on the @a crAlgorithm from its initial state. 
  **
  ** @param crAlgorithm : CRAlgorithm to run.
  ** @param statistics : policy statistics container. 
  **
  ** @return a boolean.
  ** @see CRAlgorithm
  ** @see PolicyStatistics 
  */
  bool run(CRAlgorithmPtr crAlgorithm, PolicyStatisticsPtr statistics = PolicyStatisticsPtr());

  /**
  ** Runs the current policy on the @a crAlgorithm from its initial state. 
  **
  ** @param crAlgorithms : CRAlgorithm to run.
  ** @param statistics : policy statistics container. 
  ** @param progress : progress bar callback.
  **
  ** @return a boolean.
  ** @see ObjectStream
  ** @see PolicyStatistics
  */
  bool run(ObjectStreamPtr crAlgorithms, PolicyStatisticsPtr statistics = PolicyStatisticsPtr(), ProgressCallbackPtr progress = ProgressCallbackPtr());

  /**
  ** Runs the current policy on the @a crAlgorithm from its initial state. 
  **
  ** @param crAlgorithms : CRAlgorithm to run.
  ** @param statistics : policy statistics container. 
  ** @param progress : progress bar callback.
  **
  ** @return a boolean.
  ** @see ObjectContainer
  ** @see PolicyStatistics
  */
  bool run(ObjectContainerPtr crAlgorithms, PolicyStatisticsPtr statistics = PolicyStatisticsPtr(), ProgressCallbackPtr progress = ProgressCallbackPtr());

	/**
	 ** Computes policy statistics and runs the current policy.
	 **
	 ** @param crAlgorithms : CRAlgorithm to run.
	 ** @param progress : progress bar callback.
	 **
	 ** @return a PolicyStatistics instance.
	 ** @see ObjectStream
	 ** @see PolicyStatistics
	 */	
  PolicyStatisticsPtr computeStatistics(ObjectStreamPtr crAlgorithms, ProgressCallbackPtr progress = ProgressCallbackPtr())
    {PolicyStatisticsPtr res = new PolicyStatistics(); run(crAlgorithms, res, progress); return res;}

	/**
	 ** Computes policy statistics and runs the current policy.
	 **
	 ** @param crAlgorithms : CRAlgorithm to run.
	 ** @param progress : progress bar callback.
	 **
	 ** @return a boolean.
	 ** @see ObjectContainer
	 ** @see PolicyStatistics
	 */
  PolicyStatisticsPtr computeStatistics(ObjectContainerPtr crAlgorithms, ProgressCallbackPtr progress = ProgressCallbackPtr())
    {PolicyStatisticsPtr res = new PolicyStatistics(); run(crAlgorithms, res, progress); return res;}

  /**
  ** Sets the level of verbosity.
  **
  ** @param verbosity : level of verbosity.
  ** @param ostr : output stream.
  **
  ** @return a new (Verbose)Policy instance.
  */
  PolicyPtr verbose(size_t verbosity, std::ostream& ostr = std::cout) const;

public:
  /**
  ** Policy behaviour when entering into the @a crAlgorithm.
  **
  ** @param crAlgorithm : CRALgorithm.
  ** @see CRAlgorithm 
  */
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm) {}

  /**
  **
  **
  ** @param choose : choose.
  **
  ** @return
  */
  virtual VariablePtr policyChoose(ChoosePtr choose) = 0;

  /**
  **
  **
  ** @param reward :  reward value.
  */
  virtual void policyReward(double reward) {}

  /**
  ** Policy behaviour when leaving a scope.
  **
  */
  virtual void policyLeave() {}

  /**
  ** deprecated
  **
  **
  ** @return
  */
  virtual size_t getNumResults() const {return 0;}

  /**
  ** deprecated
  **
  ** @param i
  **
  ** @return
  */
  virtual ObjectPtr getResult(size_t i) const
    {assert(false); return ObjectPtr();}

  /**
  ** #FIXME
  **
  ** @param name
  **
  ** @return
  */
  virtual ObjectPtr getResultWithName(const std::string& name) const;
};

/**
** Returns a Policy that does randomly choices.
**
** @return a random Policy instance.
** @see RandomPolicy 
*/
extern PolicyPtr randomPolicy();

/**
** Returns a greedy policy instance.
**
** @param actionValues : 
**
** @return a greedy policy instance.
** @see GreedyPolicy 
*/
extern PolicyPtr greedyPolicy(ActionValueFunctionPtr actionValues);

/**
** Returns a Gibbs greedy policy instance. 
**
** @param actionValue
** @param temperature
**
** @return a Gibbs greedy policy instance. 
** @see GibbsGreedyPolicy 
*/
extern PolicyPtr gibbsGreedyPolicy(ActionValueFunctionPtr actionValue, IterationFunctionPtr temperature);

/**
** Returns a stochastic policy instance.
**
** @param actionProbabilities
**
** @return a stochastic policy instance.
** @see StochasticPolicy
*/
extern PolicyPtr stochasticPolicy(ActionValueFunctionPtr actionProbabilities);

/**
** Returns an epsilon-greedy policy instance.
**
** @param basePolicy
** @param epsilon
**
** @return an epsilon-greedy policy instance.
** @see EpsilonGreedyPolicy
*/
extern PolicyPtr epsilonGreedyPolicy(PolicyPtr basePolicy, IterationFunctionPtr epsilon);

/**
** Returns a mixture policy instance.
**
** @param policy1 : first policy.
** @param policy2 : second policy.
** @param mixtureCoefficient : probability of selecting @a policy2.
**
** @return a mixture policy instance.
** @see MixturePolicy 
*/
extern PolicyPtr mixturePolicy(PolicyPtr policy1, PolicyPtr policy2, double mixtureCoefficient = 0.5);

/**
** #FIXME
**
** @param policy : search policy.
** @param statistics :
**
** @return a
** @see ComputeStatisticsolicy 
*/
extern PolicyPtr computeStatisticsPolicy(PolicyPtr policy, PolicyStatisticsPtr statistics);

/**
** Returns a QLearning policy instance.
**
** @param explorationPolicy
** @param regressor
** @param discount
**
** @return a QLearning policy instance.
*/
extern PolicyPtr qlearningPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);

/**
**
**
** @param explorationPolicy
** @param regressor
** @param discount
**
** @return
*/
extern PolicyPtr sarsaZeroPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);

/**
**
**
** @param explorationPolicy
** @param regressor
** @param discount
**
** @return
*/
extern PolicyPtr monteCarloControlPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);

/**
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

/**
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

/**
**
**
** @param classifier
** @param beta
** @param exploration
**
** @return
*/
extern PolicyPtr gpomdpPolicy(GeneralizedClassifierPtr classifier, double beta, double exploration = 1.0);

/**
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
  /**
  **
  **
  ** @param decorated
  **
  ** @return
  */
  DecoratorPolicy(PolicyPtr decorated = PolicyPtr())
    : decorated(decorated) {}

  /**
  **
  **
  ** @param crAlgorithm
  */
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
    {decorated->policyEnter(crAlgorithm);}

  /**
  **
  **
  ** @param choose
  **
  ** @return
  */
  virtual VariablePtr policyChoose(ChoosePtr choose)
    {return decorated->policyChoose(choose);}

  /**
  **
  **
  ** @param reward
  */
  virtual void policyReward(double reward)
    {decorated->policyReward(reward);}

  /**
  **
  **
  */
  virtual void policyLeave()
    {decorated->policyLeave();}

  /**
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const
    {write(ostr, decorated);}

  /**
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

  /**
  **
  **
  ** @param ostr
  */
  virtual void save(std::ostream& ostr) const
    {write(ostr, decorated);}

  /**
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
