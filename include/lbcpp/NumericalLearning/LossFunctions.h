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

# include "../Core/Container.h"
# include "../Core/DynamicObject.h"
# include "../Learning/LossFunction.h"

namespace lbcpp
{

/*
** Regression Loss Functions
*
inline ScalarFunctionPtr oldSquareLossFunction(double target)
  {return squareFunction(addConstantScalarFunction(-target));}

inline ScalarFunctionPtr oldAbsoluteLossFunction(double target)
  {return absFunction(addConstantScalarFunction(-target));}

inline ScalarFunctionPtr oldDihedralAngleSquareLossFunction(double target)
  {return squareFunction(angleDifferenceScalarFunction(target));}

*/
/*
extern RankingLossFunctionPtr f1ScoreRankingLossFunction(DiscriminativeLossFunctionPtr baseLoss, const std::vector<double>& costs);
extern RankingLossFunctionPtr mccRankingLossFunction(DiscriminativeLossFunctionPtr baseLoss, const std::vector<double>& costs);
*/
/*
** Regularizers
*/
extern ScalarVectorFunctionPtr l2RegularizerFunction(double weight);

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_LOSS_FUNCTIONS_H_
