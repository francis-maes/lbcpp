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

class Policy : public Object
{
public:
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm) {}
  virtual const void* policyChoose(ChoosePtr choose) = 0;
  virtual void policyReward(double reward) {}
  virtual void policyLeave() {}
};

typedef ReferenceCountedObjectPtr<Policy> PolicyPtr;

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

class ComputeStatisticsPolicy : public DecoratorPolicy
{
public:
  ComputeStatisticsPolicy(PolicyPtr decorated = PolicyPtr())
    : DecoratorPolicy(decorated) {}
    
  virtual std::string toString() const
  {
    std::string res = cralgo::toString(numChooses) + " chooses, " +
        cralgo::toString(numChoices) + " choices, " +
        cralgo::toString(rewardSum) + " reward";
    if (numChooses)
    {
      res += ", " + cralgo::toString(numChoices / (double)numChooses) + " choices/choose, "
        + cralgo::toString(rewardSum / numChooses) + " reward/choose";
    }
    return res;
  }
    
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm)
  {
    numChooses = numChoices = 0;
    rewardSum = 0.0;
    DecoratorPolicy::policyEnter(crAlgorithm);
  }
  
  virtual const void* policyChoose(ChoosePtr choose)
  {
    ++numChooses;
    numChoices += choose->getNumChoices();
    return DecoratorPolicy::policyChoose(choose);
  }

  virtual void policyReward(double reward)
  {
    rewardSum += reward;
    DecoratorPolicy::policyReward(reward);
  }

private:
  size_t numChooses;
  size_t numChoices; // todo: incremental statistics
  double rewardSum;
};

class RandomPolicy : public Policy
{
public:
  virtual const void* policyChoose(ChoosePtr choose)
    {return choose->sampleRandomChoice();}
};

class GreedyPolicy : public Policy
{
public:
  GreedyPolicy(ActionValueFunctionPtr actionValue)
    : actionValue(actionValue) {}
    
  virtual const void* policyChoose(ChoosePtr choose)
    {return choose->sampleBestChoice(actionValue);}
  
protected:
  virtual void save(std::ostream& ostr) const
    {write(ostr, actionValue);}

  virtual bool load(std::istream& istr)
    {return read(istr, actionValue);}
  
private:
  ActionValueFunctionPtr actionValue;
};

class EpsilonGreedyPolicy : public DecoratorPolicy
{
public:
  EpsilonGreedyPolicy(double epsilon)
    : epsilon(epsilon) {}

  virtual const void* policyChoose(ChoosePtr choose)
  {
    const void* choice = DecoratorPolicy::policyChoose(choose);
    return Random::getInstance().sampleBool(epsilon)
      ? choose->sampleRandomChoice()
      : choice;
  }
  
  double getEpsilon() const
    {return epsilon;}
    
  double& getEpsilon()
    {return epsilon;}
  
private:
  double epsilon;
};

}; /* namespace cralgo */

#endif // !CRALGO_POLICY_H_
