/*-----------------------------------------.---------------------------------.
| Filename: ContinuousFunction.h           | Continuous functions            |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 03:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   ContinuousFunction.h
**@author Francis MAES
**@date   Fri Jun 12 17:13:40 2009
**
**@brief  Continuous functions.
**
**
*/

#ifndef LBCPP_CONTINUOUS_FUNCTION_H_
# define LBCPP_CONTINUOUS_FUNCTION_H_

# include "SparseVector.h"
# include "DenseVector.h"
# include "LearningExample.h"

namespace lbcpp
{

/*!
** @class ContinuousFunction
** @brief Continuous function class declaration.
**
*/
class ContinuousFunction : public Object
{
public:
  /*!
  ** Check if the function is derivable or not.
  **
  ** @return True if derivable.
  */
  virtual bool isDerivable() const = 0;
};


/*!
** @class ScalarFunction
** @brief f : R -> R
**
*/
class ScalarFunction : public ContinuousFunction
{
public:
  /*!
  ** Compute function value in @a input. Compute f(input).
  **
  ** @param input : function argument.
  **
  ** @return f(input) value.
  */
  virtual double compute(double input) const;

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual double computeDerivative(double input) const;

  /*!
  **
  **
  ** @param input
  ** @param direction
  **
  ** @return
  */
  virtual double computeDerivative(double input, double direction) const;

  /*!
  **
  **
  ** @param input : function argument.
  ** @param output : result container.
  ** @param derivative
  */
  virtual void compute(double input, double* output, double* derivative) const;

  /*!
  **
  **
  ** @param input : function argument.
  ** @param output : result container.
  ** @param derivativeDirection
  ** @param derivative
  */
  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const = 0;
};


/*!
** @class ScalarLossFunction
** @brief f : example x R -> R
**
*/
class ScalarLossFunction : public ScalarFunction
{
public:
  /*!
  **
  **
  ** @param learningExample
  */
  virtual void setLearningExample(const LearningExample& learningExample) = 0;
};


/*!
** @class ScalarVectorFunction
** @brief \f[ f : R^n \to R \f]
**
*/
class ScalarVectorFunction : public ContinuousFunction
{
public:
  /*!
  ** Compute f(@a input).
  **
  ** @param input : function argument.
  **
  ** @return f(input).
  */
  virtual double compute(const FeatureGeneratorPtr input) const;

  /*!
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual FeatureGeneratorPtr computeGradient(const FeatureGeneratorPtr input) const;

  /*!
  **
  **
  ** @param input : function argument.
  ** @param gradientDirection
  **
  ** @return
  */
  virtual FeatureGeneratorPtr computeGradient(const FeatureGeneratorPtr input, const FeatureGeneratorPtr gradientDirection) const;

  /*!
  **
  **
  ** @param input : function argument.
  ** @param output : result container.
  ** @param gradient
  */
  virtual void compute(const FeatureGeneratorPtr input, double* output, FeatureGeneratorPtr* gradient) const;

  /*!
  **
  **
  ** @param input : function argument.
  ** @param output : result container.
  ** @param gradientDirection
  ** @param gradient
  */
  virtual void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const = 0;

  /*!
  **
  **
  ** @param parameters
  ** @param direction
  **
  ** @return
  */
  bool checkDerivativeWrtDirection(const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction);

  /*!
  ** returns g : R -> R
  ** with g(x) = f(parameters + x * direction)
  **
  ** @param parameters
  ** @param direction
  **
  ** @return
  */
  ScalarFunctionPtr lineFunction(const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction) const;
};

/*!
**
**
** @param weight
**
** @return
*/
extern ScalarVectorFunctionPtr sumOfSquaresFunction(double weight = 1.0);

/*
** f : example x R^n -> R
*/
/*!
** @class VectorLossFunctionm
** @brief \f[ f : \text{example} x R^n \to R \f]
**
*/
class VectorLossFunction : public ScalarVectorFunction
{
public:
  /*!
  **
  **
  ** @param learningExample
  */
  virtual void setLearningExample(const LearningExample& learningExample) = 0;
};

/*
** f : params x features -> R
*/
/*!
** @class CRAlgorithm
** @brief #FIXME
**
*/
class ScalarArchitecture : public ContinuousFunction
{
public:
  // todo: non-derivable scalar architectures
  /*!
  **
  **
  ** @param inputDictionary
  **
  ** @return
  */
  virtual FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const = 0;

  /*!
  **
  **
  ** @param inputDictionary
  ** @param initializeRandomly
  **
  ** @return
  */
  DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
  {
    DenseVectorPtr res = new DenseVector(getParametersDictionary(inputDictionary));
    if (initializeRandomly)
      res->initializeRandomly();
    return res;
  }

  /*!
  **
  **
  ** @param parameters
  ** @param input
  **
  ** @return
  */
  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const = 0;

  /*!
  **
  **
  ** @param parameters
  ** @param input : function argument.
  ** @param output : result container.
  ** @param gradientWrtParameters
  ** @param gradientWrtInput
  */
  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      double* output,
      FeatureGeneratorPtr* gradientWrtParameters,
      FeatureGeneratorPtr* gradientWrtInput) const = 0;
};


/*!
** @class VectorArchitecture
** @brief f : params x features -> R^n
**
*/
class VectorArchitecture : public ContinuousFunction
{
public:
  // todo: non-derivable vector architectures
  /*!
  **
  **
  ** @param inputDictionary
  **
  ** @return
  */
  virtual FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const = 0;

  /*!
  **
  **
  ** @param inputDictionary
  ** @param initializeRandomly
  **
  ** @return
  */
  DenseVectorPtr createInitialParameters(FeatureDictionaryPtr inputDictionary, bool initializeRandomly) const
  {
    DenseVectorPtr res = new DenseVector(getParametersDictionary(inputDictionary));
    if (initializeRandomly)
      res->initializeRandomly();
    return res;
  }

  /*!
  **
  **
  ** @param parameters
  ** @param input
  **
  ** @return
  */
  virtual FeatureGeneratorPtr compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const = 0;

  /*!
  **
  **
  ** @param parameters
  ** @param input
  ** @param outputNumber
  **
  ** @return
  */
  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber) const = 0;

  /*!
  **
  **
  ** @param parameters
  ** @param input
  ** @param outputNumber
  ** @param output
  ** @param gradientWrtParameters
  ** @param gradientWrtInput
  */
  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
                size_t outputNumber, double* output,
                FeatureGeneratorPtr* gradientWrtParameters,
                FeatureGeneratorPtr* gradientWrtInput) const = 0;

  /*!
  **
  **
  ** @param parameters
  ** @param input
  ** @param output
  ** @param gradientsWrtParameters
  ** @param gradientsWrtInput
  */
  virtual void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
      FeatureGeneratorPtr* output,
      FeatureGeneratorPtr* gradientsWrtParameters,
      FeatureGeneratorPtr* gradientsWrtInput) const = 0;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CONTINUOUS_FUNCTION_H_
