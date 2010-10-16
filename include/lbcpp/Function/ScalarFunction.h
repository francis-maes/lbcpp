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

# include "Function.h"

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
  /*
  ** Function
  */
  virtual TypePtr getInputType() const
    {return doubleType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return doubleType;}

  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
    {return compute(input.getDouble());}

  /**
  ** Checks if the function is derivable or not.
  **
  ** @return True if derivable.
  */
  virtual bool isDerivable() const = 0;

  virtual double compute(double input) const;
  virtual double computeDerivative(double input) const;
  virtual double computeDerivative(double input, double direction) const;
  virtual void compute(double input, double* output, double* derivative) const;
  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const = 0;

  ScalarFunctionPtr multiplyByScalar(double scalar);
  ScalarFunctionPtr composeWith(ScalarFunctionPtr postFunction) const;
};

extern ClassPtr scalarFunctionClass;

class BinaryClassificationLossFunction : public ScalarFunction
{
public:
  BinaryClassificationLossFunction(bool isPositive) : isPositive(isPositive) {}
  BinaryClassificationLossFunction() {}

  virtual String toString() const;

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const;

protected:
  virtual void computePositive(double input, double* output, const double* derivativeDirection, double* derivative) const = 0;

  bool isPositive;
};

typedef ReferenceCountedObjectPtr<BinaryClassificationLossFunction> BinaryClassificationLossFunctionPtr;

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

// x -> x^2
extern ScalarFunctionPtr squareFunction();

// x -> f(x)^2
inline ScalarFunctionPtr squareFunction(ScalarFunctionPtr input)
  {return input->composeWith(squareFunction());}

// x -> |x|
extern ScalarFunctionPtr absFunction();

inline ScalarFunctionPtr absFunction(ScalarFunctionPtr input)
  {return input->composeWith(absFunction());}

/*
** Regression Loss Functions
*/
inline ScalarFunctionPtr squareLossFunction(double target)
  {return squareFunction(addConstantScalarFunction(-target));}

inline ScalarFunctionPtr absoluteLossFunction(double target)
  {return absFunction(addConstantScalarFunction(-target));}

inline ScalarFunctionPtr dihedralAngleSquareLossFunction(double target)
  {return squareFunction(angleDifferenceScalarFunction(target));}

/*
** Binary Classification Loss Functions
*/
extern BinaryClassificationLossFunctionPtr hingeLossFunction(bool isPositive, double margin = 1);
extern BinaryClassificationLossFunctionPtr logBinomialLossFunction(bool isPositive);

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_H_
