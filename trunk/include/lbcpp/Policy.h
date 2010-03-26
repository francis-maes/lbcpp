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
** @brief A Policy is a decision-maker in a Decision
** Process. Formally, a policy \f$ \pi \f$ can be seen as a function
** that map states to actions: \f$ \pi:S\to A \f$.
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
  ** Fonction called when the CRAlgorithm encounters a @a choose keyword.
  **
  ** @param choose : choose encountered.
  **
  ** @return
  */
  virtual VariablePtr policyChoose(ChoosePtr choose) = 0;

  /**
  ** Fonction called when the CRAlgorithm encounters a @a reward keyword.
  **
  ** @param reward : reward value.
  */
  virtual void policyReward(double reward) {}

  /**
  ** Policy behaviour when leaving the CRAlgorithm.
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
  virtual ObjectPtr getResultWithName(const String& name) const;
};

/**
** Creates a random Policy.
**
** The random policy selects actions randomly. Each action is selected
** with the same probability.
**
** @return a random Policy instance.
*/
extern PolicyPtr randomPolicy();

/**
** Creates an ActionValue based Policy.
**
** An ActionValue based policy \f$ \Pi_Q \f$ select actions that maximizes their
** value according to the action-value \f$ Q \f$:
** \f[ \forall s \in S, \Pi_Q(s)=\underset{a\in
** A}{\text{argmax}}\ Q(s,a) \f]
**
** @param actionValues : ActionValue function that computes scores
** of each choice.
**
** @return a greedy policy instance.
** @see ActionValueFunction
*/
extern PolicyPtr greedyPolicy(ActionValueFunctionPtr actionValues);

/**
** Creates an ActionValue based policy.
*
** This policy is a decorator which adds noise in the Action selection
** process. Gibbs-greedy sampling only works with policies defined
** through an ActionValue. Given the action-value \f$ Q \f$, the
** probability of selecting an action \f$ a \f$ is:
** \f[ P[a|s]=\frac{exp(Q(s,a)/T)}{Z}\f]
** where \f$ T \f$ is the temperature parameter and \f$ Z=\sum_{a\in
** A} exp(Q(s,a)/T) \f$ is the normalization coefficient.
**
** High temperatures lead to nearly-uniform distributions
** (exploration) whereas low temperatures lead to distribution that
** focus on the best values of (exploitation).
**
** @param actionValue : ActionValue function that computes scores of
** each choice.
** @param temperature : the Temperature parameter.
**
** @return a Gibbs greedy policy instance.
** @see ActionValueFunction
*/
extern PolicyPtr gibbsGreedyPolicy(ActionValueFunctionPtr actionValue, IterationFunctionPtr temperature);

/**
** Creates an ActionValue based policy.
**
** Computes the selection probabilities:
** \f[ P(s|a)=Q(s,a) \f]
**
** @param actionProbabilities : ActionValue function that computes
** selection probabilities.
**
** @return a stochastic policy instance.
** @see StochasticPolicy
*/
extern PolicyPtr stochasticPolicy(ActionValueFunctionPtr actionProbabilities);

/**
** Adds uniform noise to the current Policy.
**
** Epsilon-greedy sampling selects this Policy's action with
** probability 1 - epsilon and selects random Actions otherwise. This
** kind of sampling is often used in reinforcement-learning in order
** to balance exploration and exploitation.
**
** @param basePolicy : current Policy.
** @param epsilon : the probability of selecting actions randomly.
**
** @return a new policy decoring this one.
*/
extern PolicyPtr epsilonGreedyPolicy(PolicyPtr basePolicy, IterationFunctionPtr epsilon);

/**
** Creates a mixture of two policies.
*
** A mixture policy is parameterized by two sub-policies. It selects
** actions from its first policy with a given probability and actions
** from its second policy otherwise. Formally, given two policies \f$
** \pi_1 \f$  and \f$ \pi_2 \f$, and the probability \f$ k \f$, we
** have: \f[ \forall s \in S,
** \text{mixture}_{\pi_1,\pi_2,k}(s)=\begin{cases}\pi_1(s)&\text{with
** probility }1-k\\ \pi_2(s)&\text{with probability }k\end{cases}\f]
**
** @param policy1 : first policy.
** @param policy2 : second policy.
** @param mixtureCoefficient : probability of selecting @a policy2.
**
** @return a mixture policy instance.
*/
extern PolicyPtr mixturePolicy(PolicyPtr policy1, PolicyPtr policy2, double mixtureCoefficient = 0.5);

/**
** Decores @a policy and computes several statistics.
**
** @param policy : base policy to decore.
** @param statistics : statistics container.
**
** @return a decored policy instance.
*/
extern PolicyPtr computeStatisticsPolicy(PolicyPtr policy, PolicyStatisticsPtr statistics);

extern PolicyPtr qlearningPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);

extern PolicyPtr sarsaZeroPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);

extern PolicyPtr monteCarloControlPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);

extern PolicyPtr classificationExamplesCreatorPolicy(PolicyPtr explorationPolicy,
                        ClassifierPtr classifier,
                        ActionValueFunctionPtr supervisor = ActionValueFunctionPtr());

extern PolicyPtr rankingExamplesCreatorPolicy(PolicyPtr explorationPolicy,
                        RankerPtr ranker,
                        ActionValueFunctionPtr supervisor = ActionValueFunctionPtr());

extern PolicyPtr gpomdpPolicy(GeneralizedClassifierPtr classifier, double beta, double exploration = 1.0);

extern PolicyPtr gpomdpPolicy(GeneralizedClassifierPtr classifier, double beta, PolicyPtr explorationPolicy);

/*!
** @class DecoratorPolicy
** @brief
*/
class DecoratorPolicy : public Policy
{
public:

  DecoratorPolicy(PolicyPtr decorated = PolicyPtr())
    : decorated(decorated) {}

  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
    {decorated->policyEnter(crAlgorithm);}

  virtual VariablePtr policyChoose(ChoosePtr choose)
    {return decorated->policyChoose(choose);}


  virtual void policyReward(double reward)
    {decorated->policyReward(reward);}

  virtual void policyLeave()
    {decorated->policyLeave();}

  virtual void save(std::ostream& ostr) const
    {write(ostr, decorated);}

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

  virtual void save(std::ostream& ostr) const
    {write(ostr, decorated);}

  virtual bool load(std::istream& istr)
    {return read(istr, decorated);}

protected:
  PolicyPtr decorated;
};

}; /* namespace lbcpp */

#endif // !LBCPP_POLICY_H_
