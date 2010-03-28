/*-----------------------------------------.---------------------------------.
| Filename: QLearningPolicy.h              | QLearning/Sarsa Policy          |
| Author  : Francis Maes                   |                                 |
| Started : 13/03/2009 02:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_QLEARNING_H_
# define LBCPP_POLICY_QLEARNING_H_

# include <lbcpp/Policy.h>
# include <lbcpp/RandomVariable.h>

namespace lbcpp
{

class QLearningPolicy : public EpisodicDecoratorPolicy
{
public:  
  QLearningPolicy(PolicyPtr decorated, RegressorPtr regressor, double discount, bool useSarsaRule = false)
    : EpisodicDecoratorPolicy(decorated), regressor(regressor), discount(discount), useSarsaRule(useSarsaRule),
      predictionError(new ScalarVariableStatistics("predictionError"))
    {}

  QLearningPolicy() : discount(0.0), useSarsaRule(false) {}
  
  /*
  ** Object
  */
  virtual String toString() const
  {
    String res = useSarsaRule ? "sarsaZeroPolicy" : "qLearningPolicy";
    return res + "(" + decorated->toString() + ", " + 
      regressor->toString() + ", " + lbcpp::toString(discount) + ")";
  }
  
  virtual void save(OutputStream& ostr) const
    {EpisodicDecoratorPolicy::save(ostr); write(ostr, regressor); write(ostr, discount); write(ostr, useSarsaRule);}
    
  virtual bool load(InputStream& istr)
    {return EpisodicDecoratorPolicy::load(istr) && read(istr, regressor) && read(istr, discount) && read(istr, useSarsaRule);}

  /*
  ** EpisodicPolicy
  */
  virtual VariablePtr policyStart(ChoosePtr choose)
  {
    lastActionDescription = SparseVectorPtr();
    regressor->trainStochasticBegin();
    VariablePtr res = EpisodicDecoratorPolicy::policyStart(choose);
    lastActionDescription = choose->computeActionFeatures(res)->toSparseVector();
    return res;
  }
  
  virtual VariablePtr policyStep(double reward, ChoosePtr choose)
  {
    jassert(lastActionDescription);
    VariablePtr res = EpisodicDecoratorPolicy::policyStep(reward, choose);
    
    VariablePtr nextAction;
    if (useSarsaRule)
      nextAction = res;
    else
    {
      // todo...
    }
    
    // todo: selection of next action 
    //   qlearning: predicted, sampleBests(RegressorActionValue)
    //        -> optimize if we explore with the predicted strategy
    //   sarsa: explored, ok.
    
    SparseVectorPtr nextActionDescription = choose->computeActionFeatures(res)->toSparseVector();
    double ycorrect = reward + discount * regressor->predict(nextActionDescription);
    double ypredicted = regressor->predict(lastActionDescription);
    predictionError->push(fabs(ycorrect - ypredicted));
    regressor->trainStochasticExample(new RegressionExample(lastActionDescription, ycorrect));
    
    lastActionDescription = nextActionDescription;
    return res;
  }
  
  virtual void policyEnd(double reward)
  {
    EpisodicDecoratorPolicy::policyEnd(reward);
    if (lastActionDescription)
      regressor->trainStochasticExample(new RegressionExample(lastActionDescription, reward));
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
  bool useSarsaRule;
  ScalarVariableStatisticsPtr predictionError;
  SparseVectorPtr lastActionDescription;
};

}; /* namespace lbcpp */

#endif // !LBCPP_POLICY_QLEARNING_H_
