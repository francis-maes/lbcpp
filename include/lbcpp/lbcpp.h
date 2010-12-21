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
| Filename: lbcpp.h                        | global include file for lbcpp   |
| Author  : Francis Maes                   |                                 |
| Started : 04/01/2009 20:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LBCPP_H_
# define LBCPP_LBCPP_H_

# include "library.h"

# include "Core/Utilities.h"
# include "Core/Object.h"
# include "Core/TypeManager.h"
# include "Core/Type.h"
# include "Core/Variable.h"
# include "Core/Pair.h"
# include "Core/Library.h"
# include "Core/XmlSerialisation.h"
# include "Core/DynamicObject.h"
# include "Core/Vector.h"
# include "Core/Function.h"

# include "Data/Stream.h"
# include "Data/Consumer.h"
# include "Data/SymmetricMatrix.h"
# include "Data/RandomGenerator.h"
# include "Data/RandomVariable.h"
# include "Data/Cache.h"

# include "Network/NetworkClient.h"
# include "Network/NetworkServer.h"
# include "Network/NetworkCallback.h"

# include "Distribution/Distribution.h"
# include "Distribution/DiscreteDistribution.h"
# include "Distribution/ContinuousDistribution.h"
# include "Distribution/MultiVariateDistribution.h"

# include "Execution/ExecutionStack.h"
# include "Execution/ExecutionContext.h"
# include "Execution/ExecutionTrace.h"
# include "Execution/Notification.h"
# include "Execution/WorkUnit.h"
# include "Execution/TestUnit.h"

# include "Function/ScalarFunction.h"
# include "Function/ScalarObjectFunction.h"
# include "Function/Predicate.h"
# include "Function/IterationFunction.h"
# include "Function/StoppingCriterion.h"
# include "Function/ObjectiveFunction.h"
# include "Function/Evaluator.h"

# include "Perception/Perception.h"
# include "Perception/PerceptionRewriter.h"

# include "Optimizer/Optimizer.h"

# include "Inference/Inference.h"
# include "Inference/InferenceResultCache.h"
# include "Inference/InferenceCallback.h"
# include "Inference/DecoratorInference.h"
# include "Inference/SequentialInference.h"
# include "Inference/ParallelInference.h"
# include "Inference/InferenceBatchLearner.h"
# include "Inference/InferenceOnlineLearner.h"

# include "NumericalLearning/NumericalLearning.h"

# include "DecisionTree/DecisionTree.h"

# include "UserInterface/UserInterfaceManager.h"

#endif // !LBCPP_LBCPP_H_
