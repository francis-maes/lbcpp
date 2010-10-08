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
| Filename: ScalarObjectFunction.h         | Object -> Scalar derivable      |
| Author  : Francis Maes                   |   Function                      |
| Started : 07/03/2009 03:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_OBJECT_H_
# define LBCPP_FUNCTION_SCALAR_OBJECT_H_

# include "ScalarFunction.h"

namespace lbcpp
{

class ScalarObjectFunction;
typedef ReferenceCountedObjectPtr<ScalarObjectFunction> ScalarObjectFunctionPtr;

/**
** @class ScalarObjectFunction
** @brief \f$ f :  Object  \to  R  \f$
**
*/
class ScalarObjectFunction : public Function
{
public:
  /*
  ** Function
  */
  virtual TypePtr getInputType() const
    {return objectClass;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return doubleType;}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
    {return compute(input.getObject());}

  /**
  ** Checks if the function is derivable or not.
  **
  ** @return True if derivable.
  */
  virtual bool isDerivable() const = 0;

  virtual double compute(ObjectPtr input) const
    {double res = 0.0; compute(input, &res, NULL, 0.0); return res;}

  // if (output) *output += f(input)
  // if (gradientTarget) *gradientTarget += gradient_f(input) * gradientWeight
  virtual void compute(ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const = 0;

  ScalarObjectFunctionPtr multiplyByScalar(double weight) const;
};

extern ScalarObjectFunctionPtr binarySumScalarObjectFunction(ScalarObjectFunctionPtr f1, ScalarObjectFunctionPtr f2);
extern ScalarObjectFunctionPtr multiplyByScalarObjectFunction(ScalarObjectFunctionPtr function, double scalar);
extern ScalarObjectFunctionPtr sumOfSquaresScalarObjectFunction();

inline ScalarObjectFunctionPtr l2Regularizer(double weight)
  {return sumOfSquaresScalarObjectFunction()->multiplyByScalar(weight);}

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_OBJECT_H_
