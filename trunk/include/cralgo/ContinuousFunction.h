/*-----------------------------------------.---------------------------------.
| Filename: ContinuousFunction.h           | Continuous functions            |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 03:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_CONTINUOUS_FUNCTION_H_
# define CRALGO_CONTINUOUS_FUNCTION_H_

# include "SparseVector.h"
# include "DenseVector.h"
# include "LearningExample.h"

namespace cralgo
{

class ContinuousFunction : public Object
{
public:
  virtual bool isDerivable() const = 0;
};

/*
** f : R -> R
*/
class ScalarFunction : public ContinuousFunction
{
public:
  static ScalarFunctionPtr createVectorFunctionLine(ScalarVectorFunctionPtr function, const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction);

  virtual double compute(double input) const = 0;
  virtual double computeDerivative(double input) const = 0;
  virtual double computeDerivative(double input, double direction) const = 0;
  virtual void compute(double input, double* output, double* derivative) const = 0;
  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const = 0;
};

/*
** f : example x R -> R
*/
class ScalarLossFunction : public ScalarFunction
{
public:
  virtual void setLearningExample(const LearningExample& learningExample) = 0;
};

/*
** f : R^n -> R
*/
class ScalarVectorFunction : public ContinuousFunction
{
public:
  static ScalarVectorFunctionPtr createSumOfSquares(double weight = 1.0);

public:
  virtual double compute(const FeatureGeneratorPtr input) const = 0;
  virtual FeatureGeneratorPtr computeGradient(const FeatureGeneratorPtr input) const = 0;
  virtual FeatureGeneratorPtr computeGradient(const FeatureGeneratorPtr input, const FeatureGeneratorPtr gradientDirection) const = 0;
  virtual void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const = 0;
  virtual void compute(const FeatureGeneratorPtr input, double* output, FeatureGeneratorPtr* gradient) const = 0;

  bool checkDerivativeWrtDirection(const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction)
  {
    double dirNorm = direction->l2norm();
    double epsilon = 5e-6 / dirNorm;
    double value1 = compute(FeatureGenerator::weightedSum(parameters, 1.0, direction, -epsilon, true));
    double value2 = compute(FeatureGenerator::weightedSum(parameters, 1.0, direction, epsilon, true));
    double numericalDerivative = (value2 - value1) / (2.0 * epsilon);
    FeatureGeneratorPtr gradient = computeGradient(parameters, direction);
    double analyticDerivative = gradient->dotProduct(direction);
    Object::warning("ScalarVectorFunction::checkDerivativeWrtDirection",
      "Derivative Check: " + cralgo::toString(numericalDerivative) + " vs. " + cralgo::toString(analyticDerivative));
    return fabs(numericalDerivative - analyticDerivative) < 0.00001;
  }
};

/*
** f : example x R^n -> R
*/
class VectorLossFunction : public ScalarVectorFunction
{
public:
  virtual void setLearningExample(const LearningExample& learningExample) = 0;
};

/*
** f : params x features -> R
*/
class ScalarArchitecture : public ContinuousFunction
{
public:
  // todo: non-derivable scalar architectures
  
  virtual DenseVectorPtr createInitialParameters() const = 0;

  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const = 0;
  
  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      double* output,
      FeatureGeneratorPtr* gradientWrtParameters,
      FeatureGeneratorPtr* gradientWrtInput) const = 0;
};

/*
** f : params x features -> R^n
*/
class VectorArchitecture : public ContinuousFunction
{
public:
  // todo: non-derivable vector architectures

  virtual DenseVectorPtr createInitialParameters() const = 0;

  virtual FeatureGeneratorPtr compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const = 0;

  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber) const = 0;

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                size_t outputNumber, double* output, 
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const = 0;

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      FeatureGeneratorPtr* output,
      FeatureGeneratorPtr* gradientWrtParameters,
      FeatureGeneratorPtr* gradientWrtInput) const = 0;
};

}; /* namespace cralgo */

#endif // !CRALGO_CONTINUOUS_FUNCTION_H_
