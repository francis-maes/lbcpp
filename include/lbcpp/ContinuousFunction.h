/*-----------------------------------------.---------------------------------.
| Filename: ContinuousFunction.h           | Continuous functions            |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 03:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CONTINUOUS_FUNCTION_H_
# define LBCPP_CONTINUOUS_FUNCTION_H_

# include "SparseVector.h"
# include "DenseVector.h"
# include "LearningExample.h"

namespace lbcpp
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
  virtual double compute(double input) const;
  virtual double computeDerivative(double input) const;
  virtual double computeDerivative(double input, double direction) const;
  virtual void compute(double input, double* output, double* derivative) const;
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
  virtual double compute(const FeatureGeneratorPtr input) const;
  virtual FeatureGeneratorPtr computeGradient(const FeatureGeneratorPtr input) const;
  virtual FeatureGeneratorPtr computeGradient(const FeatureGeneratorPtr input, const FeatureGeneratorPtr gradientDirection) const;
  virtual void compute(const FeatureGeneratorPtr input, double* output, FeatureGeneratorPtr* gradient) const;
  virtual void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const = 0;

  bool checkDerivativeWrtDirection(const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction);

  // returns g : R -> R
  //   with g(x) = f(parameters + x * direction)
  ScalarFunctionPtr lineFunction(const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction) const;
};

extern ScalarVectorFunctionPtr sumOfSquaresFunction(double weight = 1.0);

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
  virtual FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const = 0;
  
  DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
  {
    DenseVectorPtr res = new DenseVector(getParametersDictionary(inputDictionary));
    if (initializeRandomly)
      res->initializeRandomly();
    return res;
  }

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
  virtual FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const = 0;

  DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
  {
    DenseVectorPtr res = new DenseVector(getParametersDictionary(inputDictionary));
    if (initializeRandomly)
      res->initializeRandomly();
    return res;
  }

  virtual FeatureGeneratorPtr compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const = 0;

  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber) const = 0;

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                size_t outputNumber, double* output, 
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const = 0;

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      FeatureGeneratorPtr* output,
      FeatureGeneratorPtr* gradientsWrtParameters,
      FeatureGeneratorPtr* gradientsWrtInput) const = 0;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CONTINUOUS_FUNCTION_H_
