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
| Filename: lbcpp.h                         | global include file for lbcpp  |
| Author  : Francis Maes                   |                                 |
| Started : 04/01/2009 20:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LBCPP_H_
# define LBCPP_LBCPP_H_

# include "Random.h"

# include "Traits.h"
# include "ContainerTraits.h"
# include "Object.h"
# include "ObjectContainer.h"
# include "ObjectStream.h"
# include "ObjectConsumer.h"

# include "ObjectGraph.h"
# include "Table.h"

# include "RandomVariable.h"
# include "FeatureGenerator.h"
# include "FeatureVisitor.h"
# include "EditableFeatureGenerator.h"
# include "SparseVector.h"
# include "DenseVector.h"
# include "LearningExample.h"
# include "ContinuousFunction.h"
# include "Optimizer.h"
# include "IterationFunction.h"
# include "LearningMachine.h"
# include "GradientBasedLearningMachine.h"

# include "CRAlgorithmScope.h"
# include "Choose.h"
# include "Callback.h"
# include "Policy.h"
# include "CRAlgorithm.h"
# include "CRAlgorithmLearner.h"

#endif // !LBCPP_LBCPP_H_
