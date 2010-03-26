/*-----------------------------------------.---------------------------------.
| Filename: MonteCarloControlPolicy.h      | Monte Carlo Control Policy      |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2009 02:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_MONTE_CARLO_CONTROL_H_
# define LBCPP_POLICY_MONTE_CARLO_CONTROL_H_

# include <lbcpp/Policy.h>
# include <lbcpp/RandomVariable.h>

namespace lbcpp
{

class MonteCarloControlPolicy : public EpisodicDecoratorPolicy
{
public:  
  MonteCarloControlPolicy(PolicyPtr decorated, RegressorPtr regressor, double discount)
    : EpisodicDecoratorPolicy(decorated), regressor(regressor), discount(discount),
      predictionError(new ScalarVariableStatistics("predictionError"))
    {}

  MonteCarloControlPolicy() : discount(0.0) {}
  
  /*
  ** Object
  */
  virtual String toString() const
  {
    return "monteCarloControlPolicy(" + decorated->toString() + ", " + 
      regressor->toString() + ", " + lbcpp::toString(discount) + ")";
  }
  
  virtual void save(std::ostream& ostr) const
    {EpisodicDecoratorPolicy::save(ostr); write(ostr, regressor); write(ostr, discount);}
    
  virtual bool load(std::istream& istr)
    {return EpisodicDecoratorPolicy::load(istr) && read(istr, regressor) && read(istr, discount);}
  
  /*
  ** EpisodicPolicy
  */
  virtual VariablePtr policyStart(ChoosePtr choose)
  {
    actionFeatures.clear();
    rewards.clear();
    VariablePtr res = EpisodicDecoratorPolicy::policyStart(choose);
    actionFeatures.push_back(choose->computeActionFeatures(res)->toSparseVector());
    return res;
  }
  
  virtual VariablePtr policyStep(double reward, ChoosePtr choose)
  {
    rewards.push_back(reward);
    VariablePtr res = EpisodicDecoratorPolicy::policyStep(reward, choose);
    actionFeatures.push_back(choose->computeActionFeatures(res)->toSparseVector());
    return res;
  }
  
  virtual void policyEnd(double reward)
  {
    rewards.push_back(reward);
    EpisodicDecoratorPolicy::policyEnd(reward);
    jassert(rewards.size() == actionFeatures.size());
    
    regressor->trainStochasticBegin(actionFeatures[0]->getDictionary());
    double R = 0.0;
    for (int i = rewards.size() - 1; i >= 0; --i)
    {
      R = R * discount + rewards[i];
      SparseVectorPtr features = actionFeatures[i];
      double prediction = regressor->predict(features);
      predictionError->push(fabs(prediction - R));
      regressor->trainStochasticExample(new RegressionExample(features, R));
    }
    regressor->trainStochasticEnd();
  }
  
  size_t getNumResults() const
    {return 1 + EpisodicDecoratorPolicy::getNumResults();}
    
  ObjectPtr getResult(size_t i) const
  {
    if (i == 0)
      return predictionError;
    else
      return EpisodicDecoratorPolicy::getResult(i - 1);
  }

private:
  RegressorPtr regressor;
  double discount;
  ScalarVariableStatisticsPtr predictionError;

  std::vector<SparseVectorPtr> actionFeatures;
  std::vector<double> rewards;
};

}; /* namespace lbcpp */

#endif // !LBCPP_POLICY_MONTE_CARLO_CONTROL_H_
