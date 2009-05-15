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
    : learningRate(learningRate), normalizeLearningRate(normalizeLearningRate), epoch(0) {}
    
  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
  {
    assert(parameters);
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
  
protected:
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;
  size_t epoch;
  
  double computeAlpha()
  {
    //std::cout << "Alpha = 1.0";
    double res = 1.0;
    if (learningRate)
    {
      //std::cout << " x " << learningRate->compute(epoch);
      res *= learningRate->compute(epoch);
    }
    if (normalizeLearningRate && meanInputSize)
    {
      //std::cout << " / " << meanInputSize;
      res /= meanInputSize;
    }
    //std::cout << " = " << res << std::endl;
    return res;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNER_STOCHASTIC_H_
