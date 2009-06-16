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
**@brief  #FIXME: all
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
** @brief
*/
class GradientBasedLearner : public Object
{
public:
  /*!
  **
  **
  ** @param parameters
  */
  void setParameters(DenseVectorPtr parameters)
    {this->parameters = parameters;}

  /*!
  **
  **
  **
  ** @return
  */
  DenseVectorPtr getParameters() const
    {return parameters;}

  /*!
  **
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
extern GradientBasedLearnerPtr batchLearner(VectorOptimizerPtr optimizer, OptimizerStoppingCriterionPtr termination);

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
