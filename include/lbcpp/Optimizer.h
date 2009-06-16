/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.h                    | Continuous Function Optimizers  |
| Author  : Francis Maes                   |                                 |
| Started : 19/03/2009 20:47               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

/*!
**@file   Optimizer.h
**@author Francis MAES
**@date   Fri Jun 12 19:16:47 2009
**
**@brief  #FIXME: all
**
**
*/

#ifndef LBCPP_OPTIMIZER_H_
# define LBCPP_OPTIMIZER_H_

# include "ContinuousFunction.h"

namespace lbcpp
{

/*!
** @class OptimizerStoppingCriterion
** @brief
*/
class OptimizerStoppingCriterion : public Object
{
public:
  /*!
  **
  **
  */
  virtual void reset() = 0;

  /*!
  **
  **
  ** @param value
  ** @param parameter
  ** @param derivative
  **
  ** @return
  */
  virtual bool isTerminated(double value, double parameter, double derivative) = 0;

  /*!
  **
  **
  ** @param value
  ** @param parameters
  ** @param gradient
  **
  ** @return
  */
  virtual bool isTerminated(double value, const FeatureGeneratorPtr parameters, const FeatureGeneratorPtr gradient) = 0;
};

enum OptimizerState
{
  optimizerError,
  optimizerContinue,
  optimizerDone,
};

/*!
**
**
** @param maxIterations
**
** @return
*/
extern OptimizerStoppingCriterionPtr maxIterationsStoppingCriterion(size_t maxIterations);

/*!
**
**
** @param tolerance
**
** @return
*/
extern OptimizerStoppingCriterionPtr averageImprovementThresholdStoppingCriterion(double tolerance);

/*!
**
**
** @param criterion1
** @param criterion2
**
** @return
*/
extern OptimizerStoppingCriterionPtr logicalOr(OptimizerStoppingCriterionPtr criterion1, OptimizerStoppingCriterionPtr criterion2);


/*!
** @class ScalarOptimizer
** @brief
*/
class ScalarOptimizer : public Object
{
public:
  /*!
  **
  **
  ** @param function
  ** @param value
  ** @param stoppingCriterion
  ** @param progress
  **
  ** @return
  */
  virtual bool optimize(ScalarFunctionPtr function, double& value, OptimizerStoppingCriterionPtr stoppingCriterion, ProgressCallbackPtr progress = ProgressCallbackPtr());

protected:
  /*!
  **
  **
  ** @param function
  ** @param parameter
  **
  ** @return
  */
  virtual bool initialize(ScalarFunctionPtr function, double parameter) = 0;

  /*!
  **
  **
  ** @param function
  ** @param parameter
  ** @param value
  ** @param derivative
  **
  ** @return
  */
  virtual OptimizerState step(ScalarFunctionPtr function, double& parameter, double value, double derivative) = 0;
};


/*!
** @class VectorOptimizer
** @brief
*/
class VectorOptimizer : public Object
{
public:
  /*!
  **
  **
  ** @param function
  ** @param parameters
  ** @param stoppingCriterion
  ** @param progress
  **
  ** @return
  */
  virtual bool optimize(ScalarVectorFunctionPtr function, FeatureGeneratorPtr& parameters, OptimizerStoppingCriterionPtr stoppingCriterion, ProgressCallbackPtr progress = ProgressCallbackPtr());

  /*!
  **
  **
  ** @param function
  ** @param stoppingCriterion
  ** @param progress
  **
  ** @return
  */
  FeatureGeneratorPtr optimize(ScalarVectorFunctionPtr function, OptimizerStoppingCriterionPtr stoppingCriterion, ProgressCallbackPtr progress = ProgressCallbackPtr())
  {
    FeatureGeneratorPtr res = new DenseVector();
    if (!optimize(function, res, stoppingCriterion, progress))
      return FeatureGeneratorPtr();
    return res;
  }

protected:
  size_t iteration;             /*!< */
  ScalarVectorFunctionPtr function; /*!< */

  FeatureGeneratorPtr parameters; /*!< */
  double value;                 /*!< */
  FeatureGeneratorPtr gradientDirection; /*!< */
  FeatureGeneratorPtr gradient; /*!< */

  /*!
  **
  **
  ** @param parameters
  */
  void setParameters(FeatureGeneratorPtr parameters)
  {
    this->parameters = parameters;
    gradient = FeatureGeneratorPtr();
    function->compute(parameters, &value, gradientDirection, &gradient);
    assert(gradient->getDictionary() == parameters->getDictionary());
  }

  /*!
  **
  **
  ** @param parameters
  ** @param gradient
  ** @param value
  */
  void setParametersGradientAndValue(FeatureGeneratorPtr parameters, FeatureGeneratorPtr gradient, double value)
    {this->parameters = parameters; this->value = value; this->gradient = gradient;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual bool initialize()   {return true;}

  /*!
  **
  **
  **
  ** @return
  */
  virtual OptimizerState step() = 0;
};

/*!
**
**
** @param stepSize
**
** @return
*/
extern VectorOptimizerPtr gradientDescentOptimizer(IterationFunctionPtr stepSize);

/*!
**
**
**
** @return
*/
extern VectorOptimizerPtr rpropOptimizer();

/*!
**
**
**
** @return
*/
extern VectorOptimizerPtr lbfgsOptimizer();

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_H_

