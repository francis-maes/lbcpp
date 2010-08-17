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
| Filename: StoppingCriterion.h            | Stopping Criterion              |
| Author  : Francis Maes                   |                                 |
| Started : 16/06/2009 12:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_STOPPING_CRITERION_H_
# define LBCPP_STOPPING_CRITERION_H_

# include "../ObjectPredeclarations.h"

namespace lbcpp
{

/*!
** @class StoppingCriterion
** @brief
*/
class StoppingCriterion : public Object
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
  **
  ** @return
  */
  virtual bool shouldOptimizerStop(double value) = 0;
};

extern StoppingCriterionPtr maxIterationsStoppingCriterion(size_t maxIterations);
extern StoppingCriterionPtr maxIterationsWithoutImprovementStoppingCriterion(size_t maxIterationsWithoutImprovement);
extern StoppingCriterionPtr averageImprovementStoppingCriterion(double tolerance, bool relativeImprovment = false);
extern StoppingCriterionPtr logicalOr(StoppingCriterionPtr criterion1, StoppingCriterionPtr criterion2);

}; /* namespace lbcpp */

#endif // !LBCPP_STOPPING_CRITERION_H_

