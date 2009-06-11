/*-----------------------------------------.---------------------------------.
| Filename: Policy.h                       | CR-algorithm policy             |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 17:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_POLICY_H_
# define LBCPP_POLICY_H_

# include "ObjectPredeclarations.h"
# include "RandomVariable.h" // only for IterationFuction

namespace lbcpp
{

class Policy : public Object
{
public:
  bool run(CRAlgorithmPtr crAlgorithm);
  bool run(ObjectStreamPtr crAlgorithms, ProgressCallbackPtr progress = ProgressCallbackPtr());
  bool run(ObjectContainerPtr crAlgorithms, ProgressCallbackPtr progress = ProgressCallbackPtr());

  PolicyPtr addComputeStatistics() const;
  PolicyPtr verbose(size_t verbosity, std::ostream& ostr = std::cout) const;

public:
  virtual void policyEnter(CRAlgorithmPtr crAlgorithm) {}
  virtual VariablePtr policyChoose(ChoosePtr choose) = 0;
  virtual void policyReward(double reward) {}
  virtual void policyLeave() {}
  
  virtual size_t getNumResults() const {return 0;}
  virtual ObjectPtr getResult(size_t i) const
    {assert(false); return ObjectPtr();}
  
  virtual ObjectPtr getResultWithName(const std::string& name) const;
};

extern PolicyPtr randomPolicy();
extern PolicyPtr greedyPolicy(ActionValueFunctionPtr actionValues);
extern PolicyPtr gibbsGreedyPolicy(ActionValueFunctionPtr actionValue, IterationFunctionPtr temperature);
extern PolicyPtr stochasticPolicy(ActionValueFunctionPtr actionProbabilities);
extern PolicyPtr epsilonGreedyPolicy(PolicyPtr basePolicy, IterationFunctionPtr epsilon);

// mixtureCoefficient = Probability of selecting policy2
extern PolicyPtr mixturePolicy(PolicyPtr policy1, PolicyPtr policy2, double mixtureCoefficient = 0.5);

extern PolicyPtr qlearningPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);
extern PolicyPtr sarsaZeroPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);
extern PolicyPtr monteCarloControlPolicy(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount);

extern PolicyPtr classificationExampleCreatorPolicy(PolicyPtr explorationPolicy,
                        ClassifierPtr classifier,
                        ActionValueFunctionPtr supervisor = ActionValueFunctionPtr());
                        
extern PolicyPtr rankingExampleCreatorPolicy(PolicyPtr explorationPolicy,
                        RankerPtr ranker,
                        ActionValueFunctionPtr supervisor = ActionValueFunctionPtr());
                        
extern PolicyPtr gpomdpPolicy(GeneralizedClassifierPtr classifier, double beta, double exploration = 1.0);
extern PolicyPtr gpomdpPolicy(GeneralizedClassifierPtr classifier, double beta, PolicyPtr explorationPolicy);

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

}; /* namespace lbcpp */

#endif // !LBCPP_POLICY_H_
