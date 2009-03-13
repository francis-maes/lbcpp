/*-----------------------------------------.---------------------------------.
| Filename: Policy.h                       | CR-algorithm policy             |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 17:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef CRALGO_POLICY_H_
# define CRALGO_POLICY_H_

# include "ObjectPredeclarations.h"
# include "RandomVariable.h" // only for IterationFuction

namespace cralgo
{

class Policy : public Object
{
public:
  static PolicyPtr createRandom();
  static PolicyPtr createGreedy(ActionValueFunctionPtr actionValues);

  static PolicyPtr createQLearning(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);
  static PolicyPtr createSarsaZero(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);

  static PolicyPtr createClassificationExampleCreator(PolicyPtr explorationPolicy,
                        ClassifierPtr classifier,
                        ActionValueFunctionPtr supervisor = ActionValueFunctionPtr());
                        
  static PolicyPtr createRankingExampleCreator(PolicyPtr explorationPolicy,
                        RankerPtr ranker,
                        ActionValueFunctionPtr supervisor = ActionValueFunctionPtr());

  PolicyPtr epsilonGreedy(IterationFunctionPtr epsilon) const;
  PolicyPtr addComputeStatistics() const;
  PolicyPtr verbose(std::ostream& ostr, size_t verbosity) const;

public:
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm) {}
  virtual VariablePtr policyChoose(ChoosePtr choose) = 0;
  virtual void policyReward(double reward) {}
  virtual void policyLeave() {}
  
  virtual size_t getNumResults() const {return 0;}
  virtual ObjectPtr getResult(size_t i) const
    {assert(false); return ObjectPtr();}
};

template<>
struct Traits<PolicyPtr> : public ObjectPtrTraits<Policy> {};

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
    
protected:
  PolicyPtr decorated;
};

}; /* namespace cralgo */

#endif // !CRALGO_POLICY_H_
