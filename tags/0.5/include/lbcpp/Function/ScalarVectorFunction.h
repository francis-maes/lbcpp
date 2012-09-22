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
| Filename: ScalarVectorFunction.h         | Object -> Scalar derivable      |
| Author  : Francis Maes                   |   Function                      |
| Started : 07/03/2009 03:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_SCALAR_VECTOR_H_
# define LBCPP_FUNCTION_SCALAR_VECTOR_H_

# include "ScalarFunction.h"
# include "../Data/DoubleVector.h"

namespace lbcpp
{

class ScalarVectorFunction;
typedef ReferenceCountedObjectPtr<ScalarVectorFunction> ScalarVectorFunctionPtr;

/**
** @class ScalarVectorFunction
** @brief \f$ f :  R^n  \to  R  \f$
**
*/
class ScalarVectorFunction : public Function
{
public:
  virtual bool isDerivable() const = 0;

  // if (output) *output += result
  // if (gradientTarget) *gradientTarget += resultGradinet * gradientWeight
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const = 0;

  /*
  ** Function
  */
  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual String getOutputPostFix() const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const;
};

extern ClassPtr scalarVectorFunctionClass;

extern ScalarVectorFunctionPtr binarySumScalarVectorFunction(ScalarVectorFunctionPtr f1, ScalarVectorFunctionPtr f2);
extern ScalarVectorFunctionPtr multiplyByScalarVectorFunction(ScalarVectorFunctionPtr function, double scalar);

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_SCALAR_VECTOR_H_
