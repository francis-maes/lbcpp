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
  StochasticGradientDescentLearner(const StochasticGradientDescentLearner& other)
    : epoch(other.epoch), learningRate(other.learningRate), normalizeLearningRate(other.normalizeLearningRate) {}
  StochasticGradientDescentLearner() : epoch(0), normalizeLearningRate(false) {}
    
  virtual String toString() const
    {return "StochasticGradientDescentLearner(" + lbcpp::toString(learningRate) + ")";}

  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
  {
    jassert(parameters);
    if (normalizeLearningRate)
    {
      // computing the l0norm() may be long, so we make more and more sparse sampling of this quantity
      if (inputSize.getCount() < 10 ||                         // every time until having 10 samples
          (inputSize.getCount() < 100 && (epoch % 10 == 0)) || // every 10 epochs until having 100 samples
          (epoch % 100 == 0))                                  // every 100 epochs after that
      {
        inputSize.push((double)(gradient->l0norm()));
        //std::cout << "Alpha: " << weight * computeAlpha() << " inputSize: " << inputSize.toString() << std::endl;
      }
    }
    
    //std::cout << "GRADIENT ...." << std::endl << gradient->toString() << std::endl;
    //std::cout << "======================EPOCH " << epoch << " =========================================" << std::endl
    //   << "Params.addWeighted(" << gradient->l2norm() << " , " << (-weight * computeAlpha()) << ")" << std::endl;
    parameters->addWeighted(gradient, -weight * computeAlpha());
    //std::cout << " => PARAMETERS = " << parameters->toString() << " L2norm = " << parameters->l2norm() << " num params = " << parameters->l0norm() << std::endl;
    ++epoch;
  }
  
  virtual void trainStochasticEnd()
  {
    // apply regularizer
    if (parameters && regularizer)
      parameters->addWeighted(regularizer->computeGradient(parameters), -computeAlpha());
  }
  
  virtual void save(OutputStream& ostr) const
  {
    write(ostr, epoch);
    write(ostr, learningRate);
    write(ostr, normalizeLearningRate);
    inputSize.save(ostr);
  }
  
  virtual bool load(InputStream& istr)
  {
    return read(istr, epoch) && read(istr, learningRate) &&
      read(istr, normalizeLearningRate) && inputSize.load(istr);
  }
  
  virtual ObjectPtr clone() const
    {return new StochasticGradientDescentLearner(*this);}

protected:
  size_t epoch;
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;
  ScalarVariableMean inputSize;
  
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
