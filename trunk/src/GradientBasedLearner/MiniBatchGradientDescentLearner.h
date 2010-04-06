/*-----------------------------------------.---------------------------------.
| Filename: MiniBatchGradientBasedLearner.h| Mini Batch Gradient Descent     |
| Author  : Francis Maes                   |                                 |
| Started : 06/04/2010 20:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GRADIENT_BASED_LEARNER_MINI_BATCH_H_
# define LBCPP_GRADIENT_BASED_LEARNER_MINI_BATCH_H_

# include "StochasticGradientDescentLearner.h"

namespace lbcpp
{

// batchSize: 0 => automatic: an update is performed each time that trainStochasticEnd() is called
//            n > 0 => fixed-size batch size: an update is performed every 'n' trainStochasticExample() calls

class MiniBatchGradientDescentLearner : public StochasticGradientDescentLearner
{
public:
  MiniBatchGradientDescentLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate, size_t batchSize = 0)
    : StochasticGradientDescentLearner(learningRate, normalizeLearningRate), batchSize(batchSize), sumOfWeights(0.0) {}
  MiniBatchGradientDescentLearner(const MiniBatchGradientDescentLearner& other)
    : StochasticGradientDescentLearner(other), batchSize(other.batchSize), sumOfWeights(0.0) {}
  MiniBatchGradientDescentLearner() : batchSize(0), sumOfWeights(0.0) {}
    
  virtual String toString() const
    {return T("MiniBatchGradientDescentLearner(") + lbcpp::toString(learningRate) + T(", ") + lbcpp::toString(batchSize) + T(")");}

  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight)
  {
    jassert(parameters);
    if (normalizeLearningRate)
      inputSize.push((double)(gradient->l0norm()));
    
    if (!sumOfGradients)
    {
      sumOfGradients = gradient->toSparseVector();
      if (weight != 1.0)
        sumOfGradients->multiplyByScalar(weight);
    }
    else
      gradient->addWeightedTo(sumOfGradients, weight);
    sumOfWeights += weight;

    ++epoch;

    if (batchSize && ((epoch % batchSize) == 0))
      doUpdate();
  }

  void doUpdate()
  {
    //std::cout << "SumOfGradients L0 : " << sumOfGradients->l0norm() << " L1: " << sumOfGradients->l1norm() << " SumOfWeights = " << sumOfWeights << std::endl;
    //std::cout << "GRADIENT ...." << std::endl << sumOfGradients->toString() << std::endl;
    //std::cout << "Params.addWeighted(" << sumOfGradients->toString() << " , " << (- computeAlpha() / sumOfWeights) << ")" << std::endl;
    if (sumOfWeights && sumOfGradients)
      parameters->addWeighted((FeatureGeneratorPtr)sumOfGradients, -computeAlpha());
    //std::cout << " => " << parameters->toString() << std::endl;

    // apply regularizer
    if (regularizer)
      parameters->addWeighted(regularizer->computeGradient(parameters), -computeAlpha());

    //std::cout << "Parameters: L0 : " << parameters->l0norm() << " L1: " << parameters->l1norm() << std::endl;
    sumOfGradients = SparseVectorPtr();
    sumOfWeights = 0.0;
  }

  virtual void trainStochasticEnd()
  {
    if (batchSize == 0)
      doUpdate(); // auto batch size
    StochasticGradientDescentLearner::trainStochasticEnd();
  }
  
  virtual void save(OutputStream& ostr) const
  {
    StochasticGradientDescentLearner::save(ostr);
    write(ostr, batchSize);
  }
  
  virtual bool load(InputStream& istr)
    {return StochasticGradientDescentLearner::load(istr) && read(istr, batchSize);}
  
  virtual ObjectPtr clone() const
    {return new MiniBatchGradientDescentLearner(*this);}

protected:
  size_t batchSize;
  SparseVectorPtr sumOfGradients;
  double sumOfWeights;
};

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNER_MINI_BATCH_H_
