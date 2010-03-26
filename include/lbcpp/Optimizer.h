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

# include "StoppingCriterion.h"
# include "ContinuousFunction.h"

namespace lbcpp
{

enum OptimizerState
{
  optimizerError,
  optimizerContinue,
  optimizerDone,
};

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
  virtual bool optimize(ScalarFunctionPtr function, double& value, StoppingCriterionPtr stoppingCriterion, ProgressCallbackPtr progress = ProgressCallbackPtr());

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
  virtual bool optimize(ScalarVectorFunctionPtr function, FeatureGeneratorPtr& parameters, StoppingCriterionPtr stoppingCriterion, ProgressCallbackPtr progress = ProgressCallbackPtr());

  /*!
  **
  **
  ** @param function
  ** @param stoppingCriterion
  ** @param progress
  **
  ** @return
  */
  FeatureGeneratorPtr optimize(ScalarVectorFunctionPtr function, StoppingCriterionPtr stoppingCriterion, ProgressCallbackPtr progress = ProgressCallbackPtr())
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
    jassert(gradient->getDictionary() == parameters->getDictionary());
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

