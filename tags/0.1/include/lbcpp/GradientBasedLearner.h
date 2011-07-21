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
| Filename: GradientBasedLearner.h         | Gradient-based learner          |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 22:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   GradientBasedLearner.h
**@author Francis MAES
**@date   Fri Jun 12 18:06:36 2009
**
**@brief  Gradient-based learner.
**
**
*/

#ifndef LBCPP_GRADIENT_BASED_LEARNER_H_
# define LBCPP_GRADIENT_BASED_LEARNER_H_

# include "ContinuousFunction.h"
# include "IterationFunction.h"

namespace lbcpp
{

/*!
** @class GradientBasedLearner
** @brief Gradient-based learner.
*/
class GradientBasedLearner : public Object
{
public:
  /*!
  ** Parameters setter.
  **
  ** @param parameters : dense vector pointer of gradient-based learner parameters.
  */
  void setParameters(DenseVectorPtr parameters)
    {this->parameters = parameters;}

  /*!
  ** Parameters getter.
  **
  ** @return a dense vector instance of gradient-based learner parameters.
  */
  DenseVectorPtr getParameters() const
    {return parameters;}

  /*!
  ** #FIXME
  **
  ** @param regularizer
  */
  void setRegularizer(ScalarVectorFunctionPtr regularizer)
    {this->regularizer = regularizer;}

  /*!
  **
  **
  ** @param inputDictionary
  */
  virtual void trainStochasticBegin(FeatureDictionaryPtr inputDictionary) {}

  /*!
  **
  **
  ** @param gradient
  ** @param weight
  */
  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight) = 0;

  /*!
  **
  **
  ** @param example
  ** @param exampleLoss
  */
  virtual void trainStochasticExample(ObjectPtr example, ScalarVectorFunctionPtr exampleLoss)
    {trainStochasticExample(exampleLoss->computeGradient(parameters), 1.0);}

  /*!
  **
  **
  */
  virtual void trainStochasticEnd() {}

  /*!
  **
  **
  ** @param objective
  ** @param numExamples
  ** @param progress
  **
  ** @return
  */
  virtual bool trainBatch(ScalarVectorFunctionPtr objective, size_t numExamples, ProgressCallbackPtr progress)
  {
    error("GradientBasedLearner::trainBatch", "This is a non-batch learner");
    return false;
  }

protected:
  DenseVectorPtr parameters;    /*!< */
  ScalarVectorFunctionPtr regularizer; /*!< */
};

/*!
**
**
** @param normalizeLearningRate
**
** @return
*/
extern GradientBasedLearnerPtr stochasticDescentLearner(IterationFunctionPtr learningRate = IterationFunctionPtr(), bool normalizeLearningRate = true);

/*!
**
**
** @param optimizer
** @param termination
**
** @return
*/
extern GradientBasedLearnerPtr batchLearner(VectorOptimizerPtr optimizer, StoppingCriterionPtr termination);

/*!
**
**
** @param optimizer
** @param maxIterations
** @param tolerance
**
** @return
*/
extern GradientBasedLearnerPtr batchLearner(VectorOptimizerPtr optimizer, size_t maxIterations = 100, double tolerance = 0.0001);

/*!
**
**
**
** @return
*/
extern GradientBasedLearnerPtr dummyLearner();

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNER_H_