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

# include "Object/Traits.h"
# include "Object/ContainerTraits.h"
# include "Object/Object.h"
# include "Object/ObjectContainer.h"
# include "Object/ObjectStream.h"
# include "Object/ObjectConsumer.h"
# include "Object/ObjectPair.h"
# include "Object/ObjectGraph.h"
# include "Object/Table.h"
# include "Object/StringToObjectMap.h"

# include "RandomGenerator.h"
# include "RandomVariable.h"
# include "LearningExample.h"
# include "IterationFunction.h"
# include "LearningMachine.h"
# include "GradientBasedLearningMachine.h"

# include "FeatureGenerator/FeatureGenerator.h"
# include "FeatureGenerator/FeatureVisitor.h"
# include "FeatureGenerator/EditableFeatureGenerator.h"
# include "FeatureGenerator/SparseVector.h"
# include "FeatureGenerator/DenseVector.h"
# include "FeatureGenerator/ContinuousFunction.h"
# include "FeatureGenerator/Optimizer.h"

# include "CRAlgorithm/CRAlgorithmScope.h"
# include "CRAlgorithm/Choose.h"
# include "CRAlgorithm/Callback.h"
# include "CRAlgorithm/Policy.h"
# include "CRAlgorithm/CRAlgorithm.h"
# include "CRAlgorithm/CRAlgorithmLearner.h"

#endif // !LBCPP_LBCPP_H_
