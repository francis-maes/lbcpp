/*-----------------------------------------.---------------------------------.
| Filename: Policy.h                       | CR-algorithm policy             |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 17:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef CRALGO_POLICY_H_
# define CRALGO_POLICY_H_

# include "Object.h"

namespace cralgo
{

class Policy;
typedef ReferenceCountedObjectPtr<Policy> PolicyPtr;
class Classifier;
typedef ReferenceCountedObjectPtr<Classifier> ClassifierPtr;

class Policy : public Object
{
public:
  static PolicyPtr createRandom();
  static PolicyPtr createGreedy(ActionValueFunctionPtr actionValues);

  static PolicyPtr createClassificationExampleCreator(PolicyPtr explorationPolicy,
                        ClassifierPtr classifier,
                        ActionValueFunctionPtr supervisor = ActionValueFunctionPtr());

  PolicyPtr epsilonGreedy(IterationFunctionPtr epsilon) const;
  PolicyPtr addComputeStatistics() const;

public:
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm) {}
  virtual const void* policyChoose(ChoosePtr choose) = 0;
  virtual void policyReward(double reward) {}
  virtual void policyLeave() {}
};

template<>
struct Traits<PolicyPtr> : public ObjectSharedPtrTraits<Policy> {};

class DecoratorPolicy : public Policy
{
public:
  DecoratorPolicy(PolicyPtr decorated = PolicyPtr())
    : decorated(decorated) {}

  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
    {decorated->policyEnter(crAlgorithm);}
    
  virtual const void* policyChoose(ChoosePtr choose)
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
