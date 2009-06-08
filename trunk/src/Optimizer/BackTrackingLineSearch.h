/*-----------------------------------------.---------------------------------.
| Filename: BackTrackingLineSearch.h       | Backtracking line search        |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 17:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_BACKTRACKING_LINE_SEARCH_H_
# define LBCPP_OPTIMIZER_BACKTRACKING_LINE_SEARCH_H_

# include <lbcpp/Optimizer.h>
# include <lbcpp/IterationFunction.h>

namespace lbcpp
{

class BackTrackingLineSearch : public Object
{
public:
  virtual FeatureGeneratorPtr computeSuccessorPoint(const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction, double alpha)
    {return weightedSum(parameters, 1.0, direction, alpha, true);}

  bool search(ScalarVectorFunctionPtr energy, const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr gradient, const FeatureGeneratorPtr direction, double& value,
              bool isFirstIteration, FeatureGeneratorPtr& resParameters, FeatureGeneratorPtr& resGradient)
  {
    // // /dot product entre FeatureGenerator
    double origDirDeriv = direction->dotProduct(gradient);
    if (origDirDeriv >= 0) 
    {
      error("BackTrackingLineSearch::search", "Non-descent direction in backtracking line search: check your gradient!");
      energy->checkDerivativeWrtDirection(parameters, direction);
      return false;
    }

    double alpha = 1.0;
    double backoff = 0.5;
    if (isFirstIteration)
    {
      //alpha = 0.1;
      //backoff = 0.5;
      double normDir = direction->l2norm();
      assert(normDir);
      alpha = (1 / normDir);
      backoff = 0.1;
    }

    const double c1 = 1e-4;
    double oldValue = value;

    int maxIterations = 50;
    int i;
    //std::cout << "backtrack old value = " << oldValue << std::endl;
    for (i = 0; i < maxIterations; ++i)
    {
      resParameters = computeSuccessorPoint(parameters, direction, alpha);
      resGradient = FeatureGeneratorPtr();
      energy->compute(resParameters, &value, direction, &resGradient);

      //std::cout << "backtrack: " << (const char* )resParameters->getShortDescription() << " alpha = " << alpha << std::endl;
      //std::cout << "   energy = " << value << " gradient = " << (const char* )resGradient->getShortDescription() << std::endl;
      if (value <= oldValue + c1 * origDirDeriv * alpha)
        break;
      alpha *= backoff;
    }
    if (i == maxIterations)
    {
      error("BackTrackingLineSearch::search", "Backtracking: warning max backtrackline search performed. Derivative = " + lbcpp::toString(origDirDeriv));
      std::cerr << "Parameters = " << parameters->toString() << std::endl;
      std::cerr << "Gradient = " << gradient->toString() << std::endl;
      std::cerr << "Direction = " << direction->toString() << std::endl;
    }
    return true;
  }
};
/*
class L1RegularizedBackTrackingLineSearch : public BackTrackingLineSearch
{
public:
  virtual FeatureGeneratorPtr computeSuccessorPoint(const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction, double alpha)
  {
    DenseVectorPtr denseParameters = new DenseVector(*parameters->toDenseVector());
    weightedAdditionClip(denseParameters, direction, alpha);
    return denseParameters;
  }
  
  void weightedAdditionClip(DenseVectorPtr parameters, DenseVectorPtr direction, double alpha)
  {
    for (size_t i = 0; i < direction->getNumValues(); ++i)
    {
      double previousValue = parameters->get(i);
      double newValue = previousValue + direction->get(i) * alpha;
      parameters->set(i, previousValue * newValue > 0 ? newValue : 0.0);
    }
    for (size_t i = 0; i < direction->getNumSubVectors(); ++i)
    {
      DenseVectorPtr subDirection = direction->getSubVector(i);
      if (subDirection)
      {
        DenseVectorPtr& subParameters = parameters->getSubVector(i);
        if (!subParameters)
          subParameters = new DenseVector(subDirection->getDictionary());
        weightedAdditionClip(subParameters, subDirection, alpha);
      }
    }
  }
};*/

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_BACKTRACKING_LINE_SEARCH_H_
