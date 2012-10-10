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

class SolutionContainer;
typedef ReferenceCountedObjectPtr<SolutionContainer> SolutionContainerPtr;

class SolutionVector;
typedef ReferenceCountedObjectPtr<SolutionVector> SolutionVectorPtr;

class ParetoFront;
typedef ReferenceCountedObjectPtr<ParetoFront> ParetoFrontPtr;

class Solver;
typedef ReferenceCountedObjectPtr<Solver> SolverPtr;

class IterativeSolver;
typedef ReferenceCountedObjectPtr<IterativeSolver> IterativeSolverPtr;

class PopulationBasedSolver;
typedef ReferenceCountedObjectPtr<PopulationBasedSolver> PopulationBasedSolverPtr;

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

class ExpressionRPNSequence;
typedef ReferenceCountedObjectPtr<ExpressionRPNSequence> ExpressionRPNSequencePtr;

class ExpressionRPNTypeState;
typedef ReferenceCountedObjectPtr<ExpressionRPNTypeState> ExpressionRPNTypeStatePtr;

class ExpressionRPNTypeSpace;
typedef ReferenceCountedObjectPtr<ExpressionRPNTypeSpace> ExpressionRPNTypeSpacePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_ML_PREDECLARATIONS_H_
