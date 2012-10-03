/*-----------------------------------------.---------------------------------.
| Filename: predeclarations.h              | lbcpp-ml predeclarations        |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_PREDECLARATIONS_H_
# define LBCPP_ML_PREDECLARATIONS_H_

# include <lbcpp/Core/Object.h>

namespace lbcpp
{

class Domain;
typedef ReferenceCountedObjectPtr<Domain> DomainPtr;

class ContinuousDomain;
typedef ReferenceCountedObjectPtr<ContinuousDomain> ContinuousDomainPtr;

class FitnessLimits;
typedef ReferenceCountedObjectPtr<FitnessLimits> FitnessLimitsPtr;

class Fitness;
typedef ReferenceCountedObjectPtr<Fitness> FitnessPtr;

class Solution;
typedef ReferenceCountedObjectPtr<Solution> SolutionPtr;

class SolutionComparator;
typedef ReferenceCountedObjectPtr<SolutionComparator> SolutionComparatorPtr;

class Problem;
typedef ReferenceCountedObjectPtr<Problem> ProblemPtr;

class SolutionSet;
typedef ReferenceCountedObjectPtr<SolutionSet> SolutionSetPtr;

class ParetoFront;
typedef ReferenceCountedObjectPtr<ParetoFront> ParetoFrontPtr;

class Optimizer;
typedef ReferenceCountedObjectPtr<Optimizer> OptimizerPtr;

class IterativeOptimizer;
typedef ReferenceCountedObjectPtr<IterativeOptimizer> IterativeOptimizerPtr;

class PopulationBasedOptimizer;
typedef ReferenceCountedObjectPtr<PopulationBasedOptimizer> PopulationBasedOptimizerPtr;

class Sampler;
typedef ReferenceCountedObjectPtr<Sampler> SamplerPtr;

class Expression;
typedef ReferenceCountedObjectPtr<Expression> ExpressionPtr;

class VariableExpression;
typedef ReferenceCountedObjectPtr<VariableExpression> VariableExpressionPtr;

class ConstantExpression;
typedef ReferenceCountedObjectPtr<ConstantExpression> ConstantExpressionPtr;

class FunctionExpression;
typedef ReferenceCountedObjectPtr<FunctionExpression> FunctionExpressionPtr;

class TestExpression;
typedef ReferenceCountedObjectPtr<TestExpression> TestExpressionPtr;

class VectorSumExpression;
typedef ReferenceCountedObjectPtr<VectorSumExpression> VectorSumExpressionPtr;

class CreateSparseVectorExpression;
typedef ReferenceCountedObjectPtr<CreateSparseVectorExpression> CreateSparseVectorExpressionPtr;

class ExpressionDomain;
typedef ReferenceCountedObjectPtr<ExpressionDomain> ExpressionDomainPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_ML_PREDECLARATIONS_H_
