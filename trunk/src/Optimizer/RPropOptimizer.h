/*-----------------------------------------.---------------------------------.
| Filename: RPropOptimizer.h               | R-Prop Optimizer                |
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2009 21:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_RPROP_H_
# define LBCPP_OPTIMIZER_RPROP_H_

# include <lbcpp/Optimizer.h>

namespace lbcpp
{

class RPropOptimizer : public VectorOptimizer
{
public:
  virtual String toString() const
    {return "RPropOptimizer";}

  virtual bool initialize()
  {
    previousGradient = new DenseVector(parameters->getDictionary());
    derivativeSpeeds = new DenseVector(parameters->getDictionary());
    return true;
  }

  virtual OptimizerState step()
  {
    DenseVectorPtr denseParameters = parameters->toDenseVector();
    DenseVectorPtr denseGradient = gradient->toDenseVector();
    updateRecursive(denseGradient, previousGradient, derivativeSpeeds, denseParameters);
    setParameters(denseParameters);
    return optimizerContinue;
  }
  
  virtual void save(std::ostream& ostr) const
    {}

  virtual bool load(std::istream& istr)
    {return true;}

private:
  DenseVectorPtr previousGradient;
  DenseVectorPtr derivativeSpeeds;
  
  void updateRecursive(DenseVectorPtr gradient, DenseVectorPtr& previousGradient, DenseVectorPtr& derivativeSpeed, DenseVectorPtr& parameters)
  {
    jassert(gradient);
    if (!previousGradient)
      previousGradient = new DenseVector(gradient->getDictionary());
    if (!derivativeSpeed)
      derivativeSpeed = new DenseVector(gradient->getDictionary());
    if (!parameters)
      parameters = new DenseVector(gradient->getDictionary());      
    for (size_t i = 0; i < gradient->getNumValues(); ++i)
      update(gradient->get(i), previousGradient->get(i), derivativeSpeed->get(i), parameters->get(i));
    for (size_t i = 0; i < gradient->getNumSubVectors(); ++i)
      if (gradient->getSubVector(i))
        updateRecursive(gradient->getSubVector(i), previousGradient->getSubVector(i), derivativeSpeed->getSubVector(i), parameters->getSubVector(i));
  }
  
  void update(double derivative, double& previousDerivative, double& derivativeSpeed, double& parameter)
  {
    static const double minSpeed = 0.000001;
    static const double defSpeed = 0.05;
    static const double maxSpeed = 50;
    
    static const double kfaster = 1.2;
    static const double kslower = 0.5;
    
    if (!derivativeSpeed)
      derivativeSpeed = defSpeed;
    
    double derivativesProduct = previousDerivative * derivative;
    //std::cout << "speed: " << d->speed << " -> ";
    if (derivativesProduct > 0)
    {
      derivativeSpeed = std::min(derivativeSpeed * kfaster, maxSpeed);
      parameter -= sign(derivative) * derivativeSpeed;
      previousDerivative = derivative;
     // std::cout << "+";
    }
    else if (derivativesProduct < 0)
    {
      derivativeSpeed = std::max(derivativeSpeed * kslower, minSpeed);
      previousDerivative = 0.0;
     // std::cout << "-";
    }
    else
    {
      parameter -= sign(derivative) * derivativeSpeed;
      previousDerivative = derivative;
    }
    //std::cout << d->speed << std::endl;
  }

  inline static double sign(double k)
    {return k < 0 ? -1 : (k > 0 ? 1 : 0);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_RPROP_H_
