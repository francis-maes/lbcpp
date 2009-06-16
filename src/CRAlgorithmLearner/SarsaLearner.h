/*-----------------------------------------.---------------------------------.
| Filename: SarsaLearner.h                 | CRAlgorithm Learner based on    |
| Author  : Francis Maes                   |   Sarsa                         |
| Started : 16/06/2009 15:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CRALGORITHM_LEARNER_SARSA_H_
# define LBCPP_CRALGORITHM_LEARNER_SARSA_H_

# include "PolicyBasedCRAlgorithmLearner.h"
# include <lbcpp/GradientBasedLearningMachine.h>

namespace lbcpp
{

class SarsaLearner : public PolicyBasedCRAlgorithmLearner
{
public:
  SarsaLearner(RegressorPtr regressor, double discount, ExplorationType exploration, IterationFunctionPtr explorationParameter, StoppingCriterionPtr stoppingCriterion)
    : PolicyBasedCRAlgorithmLearner(exploration, explorationParameter, stoppingCriterion), regressor(regressor), discount(discount)
  {
    if (!regressor)
      this->regressor = leastSquaresLinearRegressor(stochasticDescentLearner(constantIterationFunction(0.1)));
  }
  
  SarsaLearner() : discount(0.0) {}
  
  virtual std::string toString() const
    {return "sarsaLearner(" + regressor->toString() + ", " + lbcpp::toString(discount) + ", .. FIXME)";}
    
  virtual void save(std::ostream& ostr)
    {write(ostr, regressor); write(ostr, discount); PolicyBasedCRAlgorithmLearner::save(ostr);}
    
  virtual bool load(std::istream& istr)
    {return read(istr, regressor) && read(istr, discount) && PolicyBasedCRAlgorithmLearner::load(istr);}
  
protected:
  RegressorPtr regressor;
  double discount;

  virtual ActionValueFunctionPtr createLearnedActionValues() const
    {return predictedActionValues(regressor);}
    
  virtual PolicyPtr createLearnerPolicy(PolicyPtr explorationPolicy) const
    {return sarsaZeroPolicy(explorationPolicy, regressor, discount);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_CRALGORITHM_LEARNER_SARSA_H_
