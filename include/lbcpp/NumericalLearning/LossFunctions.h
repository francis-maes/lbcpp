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
| Filename: LossFunctions.h                | Numerical Learning Losses       |
| Author  : Francis Maes                   |                                 |
| Started : 21/10/2010 18:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTIONS_H_
# define LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTIONS_H_

# include "../Function/ScalarFunction.h"
# include "../Function/ScalarObjectFunction.h"

namespace lbcpp
{

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
class BinaryClassificationLossFunction : public ScalarFunction
{
public:
  BinaryClassificationLossFunction(bool isPositive) : isPositive(isPositive) {}
  BinaryClassificationLossFunction() {}

  virtual String toString() const;

  virtual void computePositive(double input, double* output, const double* derivativeDirection, double* derivative) const = 0;

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const;

protected:
  friend class BinaryClassificationLossFunctionClass;

  bool isPositive;
};

typedef ReferenceCountedObjectPtr<BinaryClassificationLossFunction> BinaryClassificationLossFunctionPtr;

extern BinaryClassificationLossFunctionPtr hingeLossFunction(bool isPositive, double margin = 1);
extern BinaryClassificationLossFunctionPtr logBinomialLossFunction(bool isPositive);

/*
** Multi Class Loss Functions
*/
class MultiClassLossFunction : public ScalarObjectFunction
{
public:
  MultiClassLossFunction(EnumerationPtr classes, size_t correctClass);
  MultiClassLossFunction() : correctClass(0) {}

  virtual String toString() const;
  virtual void compute(ObjectPtr input, double* output, ObjectPtr* gradientTarget, double gradientWeight) const;

  virtual void compute(const std::vector<double>* input, double* output, std::vector<double>* gradientTarget, double gradientWeight) const = 0;

  size_t getNumClasses() const
    {return classes->getNumElements();}

protected:
  friend class MultiClassLossFunctionClass;

  EnumerationPtr classes;
  size_t correctClass;
};

typedef ReferenceCountedObjectPtr<MultiClassLossFunction> MultiClassLossFunctionPtr;

extern MultiClassLossFunctionPtr oneAgainstAllMultiClassLossFunction(BinaryClassificationLossFunctionPtr binaryLossFunction, EnumerationPtr classes, size_t correctClass);
extern MultiClassLossFunctionPtr mostViolatedMultiClassLossFunction(BinaryClassificationLossFunctionPtr binaryLossFunction, EnumerationPtr classes, size_t correctClass);
extern MultiClassLossFunctionPtr logBinomialMultiClassLossFunction(EnumerationPtr classes, size_t correctClass);

/*
** Regularizers
*/
extern ScalarObjectFunctionPtr l2RegularizerFunction(double weight);

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTIONS_H_
