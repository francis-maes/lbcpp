/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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

/**
** @class ContinuousFunction
** @brief Continuous function class declaration.
**
*/
class ContinuousFunction : public Object
{
public:
  /**
  ** Checks if the function is derivable or not.
  **
  ** @return True if derivable.
  */
  virtual bool isDerivable() const = 0;
};


/**
** @class ScalarFunction
** @brief \f$ f :  R  \to  R  \f$
**
*/
class ScalarFunction : public ContinuousFunction
{
public:
  ScalarFunctionPtr composeWith(ScalarFunctionPtr postFunction) const;

  /**
  ** Computes function value in @a input. Computes f(input).
  **
  ** @param input : function argument.
  **
  ** @return f(input) value.
  */
  virtual double compute(double input) const;

  /**
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual double computeDerivative(double input) const;

  /**
  **
  **
  ** @param input
  ** @param direction
  **
  ** @return
  */
  virtual double computeDerivative(double input, double direction) const;

  /**
  **
  **
  ** @param input : function argument.
  ** @param output : result container.
  ** @param derivative
  */
  virtual void compute(double input, double* output, double* derivative) const;

  /**
  **
  **
  ** @param input : function argument.
  ** @param output : result container.
  ** @param derivativeDirection
  ** @param derivative
  */
  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const = 0;
};

// x -> f(x) + constant
extern ScalarFunctionPtr sum(ScalarFunctionPtr function, double constant);

// x -> f(x) - constant
inline ScalarFunctionPtr difference(ScalarFunctionPtr function, double constant)
  {return sum(function, -constant);}

// x -> x + constant
extern ScalarFunctionPtr addConstantScalarFunction(double constant);

// x -> angleDifference(x, reference)
extern ScalarFunctionPtr angleDifferenceScalarFunction(double reference);

// x -> x^2
extern ScalarFunctionPtr squareFunction();

// x -> f(x)^2
inline ScalarFunctionPtr squareFunction(ScalarFunctionPtr input)
  {return input->composeWith(squareFunction());}

/*
** Regression Loss Functions
*/
inline ScalarFunctionPtr squareLoss(double target)
  {return squareFunction(addConstantScalarFunction(-target));}

inline ScalarFunctionPtr dihedralAngleSquareLoss(double target)
  {return squareFunction(angleDifferenceScalarFunction(target));}

/*
** Binary Classification Loss Functions
*/
extern ScalarFunctionPtr hingeLoss(size_t correctClass, double margin = 1);


/**
** @class ScalarLossFunction
** @brief \f$ f : \text{example}\times R  \to  R  \f$
**
*/
class ScalarLossFunction : public ScalarFunction
{
public:
  /**
  **
  **
  ** @param learningExample
  */
  virtual void setLearningExample(const LearningExample& learningExample) = 0;
};


/**
** @class ScalarVectorFunction
** @brief \f$ f :  R^n \to  R  \f$
**
*/
class ScalarVectorFunction : public ContinuousFunction
{
public:
  /**
  ** Computes f(@a input).
  **
  ** @param input : function argument.
  **
  ** @return f(input).
  */
  virtual double compute(const FeatureGeneratorPtr input) const;

  /**
  **
  **
  ** @param input
  **
  ** @return
  */
  virtual FeatureGeneratorPtr computeGradient(const FeatureGeneratorPtr input) const;

  /**
  **
  **
  ** @param input : function argument.
  ** @param gradientDirection
  **
  ** @return
  */
  virtual FeatureGeneratorPtr computeGradient(const FeatureGeneratorPtr input, const FeatureGeneratorPtr gradientDirection) const;

  /**
  **
  **
  ** @param input : function argument.
  ** @param output : result container.
  ** @param gradient
  */
  virtual void compute(const FeatureGeneratorPtr input, double* output, FeatureGeneratorPtr* gradient) const;

  /**
  **
  **
  ** @param input : function argument.
  ** @param output : result container.
  ** @param gradientDirection
  ** @param gradient
  */
  virtual void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, FeatureGeneratorPtr* gradient) const = 0;

  /**
  **
  **
  ** @param parameters
  ** @param direction
  **
  ** @return
  */
  bool checkDerivativeWrtDirection(const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction);

  /**
  ** Returns \f$ g :  R \to R  \f$
  ** with \f$ g(x) = f(\text{parameters} + x * \text{direction}) \f$
  **
  ** @param parameters
  ** @param direction
  **
  ** @return
  */
  ScalarFunctionPtr lineFunction(const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr direction) const;
};


extern ScalarVectorFunctionPtr sum(ScalarVectorFunctionPtr f1, ScalarVectorFunctionPtr f2);
extern ScalarVectorFunctionPtr sumOfSquaresFunction(double weight = 1.0);

/**
** @class VectorLossFunctionm
** @brief \f$ f : \text{example}\times R^n \to  R  \f$
**
*/
class VectorLossFunction : public ScalarVectorFunction
{
public:
  /**
  **
  **
  ** @param learningExample
  */
  virtual void setLearningExample(const LearningExample& learningExample) = 0;
};


/**
** @class CRAlgorithm
** @brief \f$ f : \text{params}\times\text{features}\to R \f$
**
*/
class ScalarArchitecture : public ContinuousFunction
{
public:
  ScalarArchitecture() : dotProductCache(NULL) {}

  // The two following functions create functions that take parameters as inputs and compute the loss associated to these parameters
  ScalarVectorFunctionPtr makeExampleLoss(FeatureGeneratorPtr input, ScalarFunctionPtr lossFunction) const;
  ScalarVectorFunctionPtr makeEmpiricalRisk(ObjectContainerPtr examples) const;

  /**
  **
  **
  ** @param inputDictionary
  **
  ** @return
  */
  virtual FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const = 0;

  /**
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

  /**
  **
  **
  ** @param parameters
  ** @param input
  **
  ** @return
  */
  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const = 0;

  /**
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

  FeatureGenerator::DotProductCache* dotProductCache;

};

extern ScalarArchitecturePtr linearArchitecture();


/**
** @class VectorArchitecture
** @brief \f$ f : \text{params}\times\text{features}\to R^n\f$
**
*/
class VectorArchitecture : public ContinuousFunction
{
public:
  // todo: non-derivable vector architectures
  /**
  **
  **
  ** @param inputDictionary
  **
  ** @return
  */
  virtual FeatureDictionaryPtr getParametersDictionary(FeatureDictionaryPtr inputDictionary) const = 0;

  /**
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

  /**
  **
  **
  ** @param parameters
  ** @param input
  **
  ** @return
  */
  virtual FeatureGeneratorPtr compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input) const = 0;

  /**
  **
  **
  ** @param parameters
  ** @param input
  ** @param outputNumber
  **
  ** @return
  */
  virtual double compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input, size_t outputNumber) const = 0;

  /**
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

  /**
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
