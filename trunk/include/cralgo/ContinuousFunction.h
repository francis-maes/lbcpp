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
# include "LazyVector.h"
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
  virtual double compute(double input) const = 0;
  virtual double computeDerivative(double input) const = 0;
  virtual double computeDerivative(double input, double direction) const = 0;
  virtual void compute(double input, double* output, double* derivative) const = 0;
  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const = 0;
};

typedef ReferenceCountedObjectPtr<ScalarFunction> ScalarFunctionPtr;

/*
** f : example x R -> R
*/
class ScalarLossFunction : public ScalarFunction
{
public:
  virtual void setLearningExample(const LearningExample& learningExample) = 0;
};

typedef ReferenceCountedObjectPtr<ScalarLossFunction> ScalarLossFunctionPtr;

/*
** f : R^n -> R
*/
class ScalarVectorFunction : public ContinuousFunction
{
public:
  virtual double compute(const FeatureGeneratorPtr input) const = 0;
  virtual LazyVectorPtr computeGradient(const FeatureGeneratorPtr input) const = 0;
  virtual LazyVectorPtr computeGradient(const FeatureGeneratorPtr input, const FeatureGeneratorPtr gradientDirection) const = 0;

  virtual void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, LazyVectorPtr gradient) const = 0;
  virtual void compute(const FeatureGeneratorPtr input, double* output, LazyVectorPtr gradient) const = 0;
};
typedef ReferenceCountedObjectPtr<ScalarVectorFunction> ScalarVectorFunctionPtr;

/*
** f : example x R^n -> R
*/
class VectorLossFunction : public ScalarVectorFunction
{
public:
  virtual void setLearningExample(const LearningExample& learningExample) = 0;
};

typedef ReferenceCountedObjectPtr<VectorLossFunction> VectorLossFunctionPtr;

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
      LazyVectorPtr gradientWrtParameters,
      LazyVectorPtr gradientWrtInput) const = 0;
};

typedef ReferenceCountedObjectPtr<ScalarArchitecture> ScalarArchitecturePtr;


/*
** f : params x features -> R^n
*/
class VectorArchitecture : public ContinuousFunction
{
public:
  // todo: non-derivable vector architectures

  virtual DenseVectorPtr createInitialParameters() const = 0;

  virtual LazyVectorPtr compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const = 0;

  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber) const = 0;

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                size_t outputNumber, double* output, 
                LazyVectorPtr gradientWrtParameters,
                LazyVectorPtr gradientWrtInput) const = 0;

  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      LazyVectorPtr output,
      LazyVectorPtr gradientWrtParameters,
      LazyVectorPtr gradientWrtInput) const = 0;
};

typedef ReferenceCountedObjectPtr<VectorArchitecture> VectorArchitecturePtr;

}; /* namespace cralgo */

#endif // !CRALGO_CONTINUOUS_FUNCTION_H_
