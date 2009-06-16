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
  bool run(CRAlgorithmPtr crAlgorithm);

  /*!
  **
  **
  ** @param crAlgorithms
  ** @param progress
  **
  ** @return
  */
  bool run(ObjectStreamPtr crAlgorithms, ProgressCallbackPtr progress = ProgressCallbackPtr());

  /*!
  **
  **
  ** @param crAlgorithms
  ** @param progress
  **
  ** @return
  */
  bool run(ObjectContainerPtr crAlgorithms, ProgressCallbackPtr progress = ProgressCallbackPtr());

  /*!
  **
  **
  **
  ** @return
  */
  PolicyPtr addComputeStatistics() const;

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
  **
  **
  **
  ** @return
  */
  virtual size_t getNumResults() const {return 0;}

  /*!
  **
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
extern PolicyPtr classificationExampleCreatorPolicy(PolicyPtr explorationPolicy,
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
extern PolicyPtr rankingExampleCreatorPolicy(PolicyPtr explorationPolicy,
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
/*
class PolicyStatistics : public Object
{
public:
  double getRewardPerChoose() const;
  double getRewardPerChooseStddev() const;
  RandomVariableStatisticsPtr getRewardPerChooseStatistics() const;
  
  double getRewardPerEpisode() const;
  double getRewardPerEpisodeStddev() const;
  RandomVariableStatisticsPtr getRewardPerEpisodeStatistics() const;
  
  // ...
  
private:
  
};*/

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

}; /* namespace lbcpp */

#endif // !LBCPP_POLICY_H_
