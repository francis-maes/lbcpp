/*-----------------------------------------.---------------------------------.
| Filename: StochasticGradientBasedLearner.h| Stochastic Gradient Descent    |
| Author  : Francis Maes                   |                                 |
| Started : 15/05/2009 18:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNER_STOCHASTIC_H_
# define LBCPP_GRADIENT_BASED_LEARNER_STOCHASTIC_H_

# include <lbcpp/GradientBasedLearner.h>

namespace lbcpp
{

class StochasticGradientDescentLearner : public GradientBasedLearner
{
public:
  StochasticGradientDescentLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate)
    : epoch(0), learningRate(learningRate), normalizeLearningRate(normalizeLearningRate) {}
  StochasticGradientDescentLearner() : epoch(0), normalizeLearningRate(false) {}
    
  virtual std::string toString() const
    {return "StochasticGradientDescentLearner(" + lbcpp::toString(learningRate) + ")";}

  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
  {
    assert(parameters);
    if (normalizeLearningRate)
      inputSize.push((double)(gradient->l0norm()));
    
//    std::cout << "GRADIENT ...." << std::endl << gradient->toString() << std::endl;
//    std::cout << "Params.addWeighted(" << gradient->toString() << " , " << (-weight * computeAlpha()) << ")" << std::endl;
    parameters->addWeighted(gradient, -weight * computeAlpha());
//    std::cout << " => " << parameters->toString() << std::endl;
    ++epoch;
  }
  
  virtual void trainStochasticEnd()
  {
    // apply regularizer
    if (regularizer)
      parameters->addWeighted(regularizer->computeGradient(parameters), -computeAlpha());
  }
  
  virtual void save(std::ostream& ostr) const
  {
    write(ostr, epoch);
    write(ostr, learningRate);
    write(ostr, normalizeLearningRate);
    inputSize.save(ostr);
  }
  
  virtual bool load(std::istream& istr)
  {
    return read(istr, epoch) && read(istr, learningRate) &&
      read(istr, normalizeLearningRate) && inputSize.load(istr);
  }
  
protected:
  size_t epoch;
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;
  ScalarRandomVariableMean inputSize;
  
  double computeAlpha()
  {
    double res = 1.0;
    if (learningRate)
      res *= learningRate->compute(epoch);
    if (normalizeLearningRate && inputSize.getMean())
      res /= inputSize.getMean();
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNER_STOCHASTIC_H_
