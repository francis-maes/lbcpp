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

/*
** Base
*/
class Domain;
typedef ReferenceCountedObjectPtr<Domain> DomainPtr;

class DiscreteDomain;
typedef ReferenceCountedObjectPtr<DiscreteDomain> DiscreteDomainPtr;

class ContinuousDomain;
typedef ReferenceCountedObjectPtr<ContinuousDomain> ContinuousDomainPtr;

class Objective;
typedef ReferenceCountedObjectPtr<Objective> ObjectivePtr;

class DifferentiableObjective;
typedef ReferenceCountedObjectPtr<DifferentiableObjective> DifferentiableObjectivePtr;

class StochasticObjective;
typedef ReferenceCountedObjectPtr<StochasticObjective> StochasticObjectivePtr;

class FitnessLimits;
typedef ReferenceCountedObjectPtr<FitnessLimits> FitnessLimitsPtr;

class Fitness;
typedef ReferenceCountedObjectPtr<Fitness> FitnessPtr;

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

class SolverCallback;
typedef ReferenceCountedObjectPtr<SolverCallback> SolverCallbackPtr;

class Solver;
typedef ReferenceCountedObjectPtr<Solver> SolverPtr;

class IterativeSolver;
typedef ReferenceCountedObjectPtr<IterativeSolver> IterativeSolverPtr;

class PopulationBasedSolver;
typedef ReferenceCountedObjectPtr<PopulationBasedSolver> PopulationBasedSolverPtr;

class Sampler;
typedef ReferenceCountedObjectPtr<Sampler> SamplerPtr;

/*
** Search
*/
class SearchState;
typedef ReferenceCountedObjectPtr<SearchState> SearchStatePtr;

class SearchTrajectory;
typedef ReferenceCountedObjectPtr<SearchTrajectory> SearchTrajectoryPtr;

class SearchDomain;
typedef ReferenceCountedObjectPtr<SearchDomain> SearchDomainPtr;

class SearchSampler;
typedef ReferenceCountedObjectPtr<SearchSampler> SearchSamplerPtr;

class SearchAlgorithm;
typedef ReferenceCountedObjectPtr<SearchAlgorithm> SearchAlgorithmPtr;

class DecoratorSearchAlgorithm;
typedef ReferenceCountedObjectPtr<DecoratorSearchAlgorithm> DecoratorSearchAlgorithmPtr;

class SearchNode;
typedef ReferenceCountedObjectPtr<SearchNode> SearchNodePtr;

/*
** Expression
*/
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

class PostfixExpressionSequence;
typedef ReferenceCountedObjectPtr<PostfixExpressionSequence> PostfixExpressionSequencePtr;

class PostfixExpressionTypeState;
typedef ReferenceCountedObjectPtr<PostfixExpressionTypeState> PostfixExpressionTypeStatePtr;

class PostfixExpressionTypeSpace;
typedef ReferenceCountedObjectPtr<PostfixExpressionTypeSpace> PostfixExpressionTypeSpacePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_ML_PREDECLARATIONS_H_
