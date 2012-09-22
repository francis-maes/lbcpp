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
| Filename: ScalarFunction.h               | Scalar functions                |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 03:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_H_
# define LBCPP_FUNCTION_SCALAR_H_

# include "../Core/Function.h"
# include "predeclarations.h"

namespace lbcpp
{

/**
** @class ScalarFunction
** @brief \f$ f :  R  \to  R  \f$
**
*/
class ScalarFunction : public Function
{
public:
  virtual bool isDerivable() const = 0;

  // if (output) *output = result
  // if (derivative) *derivative = resultDerivative
  virtual void computeScalarFunction(double input, const Variable* otherInputs, double* output, double* derivative) const = 0;

  /*
  ** Function
  */
  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual String getOutputPostFix() const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;

  lbcpp_UseDebuggingNewOperator
};

extern ClassPtr scalarFunctionClass;

// x -> x^2
extern ScalarFunctionPtr squareFunction();

// x -> |x|
extern ScalarFunctionPtr absFunction();


#if 0
extern ScalarFunctionPtr composeScalarFunction(ScalarFunctionPtr f1, ScalarFunctionPtr f2);
extern ScalarFunctionPtr multiplyByScalarFunction(ScalarFunctionPtr function, double scalar);

// x -> f(x) + constant
extern ScalarFunctionPtr sum(ScalarFunctionPtr function, double constant);

// x -> f(x) - constant
inline ScalarFunctionPtr difference(ScalarFunctionPtr function, double constant)
  {return sum(function, -constant);}

// x -> x + constant
extern ScalarFunctionPtr addConstantScalarFunction(double constant);

// x -> angleDifference(x, reference)
extern ScalarFunctionPtr angleDifferenceScalarFunction(double reference);


// x -> f(x)^2
inline ScalarFunctionPtr squareFunction(ScalarFunctionPtr input)
  {return input->composeWith(squareFunction());}

inline ScalarFunctionPtr absFunction(ScalarFunctionPtr input)
  {return input->composeWith(absFunction());}
#endif // 0

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_H_
