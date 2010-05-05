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

#ifndef LBCPP_GRADIENT_BASED_LEARNER_H_
# define LBCPP_GRADIENT_BASED_LEARNER_H_

# include "FeatureGenerator/ContinuousFunction.h"
# include "IterationFunction.h"

namespace lbcpp
{

/*!
** @class GradientBasedLearner
** @brief Gradient-based learner.
*/
class GradientBasedLearningMachine;
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

  void setRegularizer(ScalarVectorFunctionPtr regularizer)
    {this->regularizer = regularizer;}

  virtual void trainStochasticBegin() {}
  virtual void trainStochasticExample(FeatureGeneratorPtr gradient, double weight) = 0;
  virtual void trainStochasticExample(ObjectPtr example, ScalarVectorFunctionPtr exampleLoss)
    {trainStochasticExample(exampleLoss->computeGradient(parameters), 1.0);}
  virtual void trainStochasticEnd() {}

  virtual void trainBatch(GradientBasedLearningMachine& learningMachine, ObjectContainerPtr examples, ProgressCallbackPtr progress)
  {
    error("GradientBasedLearner::trainBatch", "This is a non-batch learner");
    jassert(false);
  }

protected:
  DenseVectorPtr parameters;
  ScalarVectorFunctionPtr regularizer;
};

extern GradientBasedLearnerPtr stochasticDescentLearner(IterationFunctionPtr learningRate = IterationFunctionPtr(), bool normalizeLearningRate = true);
extern GradientBasedLearnerPtr miniBatchDescentLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate = true, size_t batchSize = 0);
extern GradientBasedLearnerPtr batchLearner(VectorOptimizerPtr optimizer, StoppingCriterionPtr termination);
extern GradientBasedLearnerPtr batchLearner(VectorOptimizerPtr optimizer, size_t maxIterations = 100, double tolerance = 0.0001);
extern GradientBasedLearnerPtr stochasticToBatchLearner(GradientBasedLearnerPtr stochasticLearner, StoppingCriterionPtr stoppingCriterion, bool randomizeExamples = true);
extern GradientBasedLearnerPtr stochasticToBatchLearner(GradientBasedLearnerPtr stochasticLearner, size_t maxIterationsWithoutImprovement = 3, size_t maxIterations = 500, bool randomizeExamples = true);

extern GradientBasedLearnerPtr dummyLearner();

}; /* namespace lbcpp */

#endif // !LBCPP_GRADIENT_BASED_LEARNER_H_
