/*-----------------------------------------.---------------------------------.
| Filename: predeclarations.h              | lbcpp-ml predeclarations        |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_PREDECLARATIONS_H_
# define ML_PREDECLARATIONS_H_

namespace lbcpp
{

/*
** Base
*/
class Domain;
typedef ReferenceCountedObjectPtr<Domain> DomainPtr;

class DiscreteDomain;
typedef ReferenceCountedObjectPtr<DiscreteDomain> DiscreteDomainPtr;

class ScalarDomain;
typedef ReferenceCountedObjectPtr<ScalarDomain> ScalarDomainPtr;

class ScalarVectorDomain;
typedef ReferenceCountedObjectPtr<ScalarVectorDomain> ScalarVectorDomainPtr;

class Objective;
typedef ReferenceCountedObjectPtr<Objective> ObjectivePtr;

class DifferentiableObjective;
typedef ReferenceCountedObjectPtr<DifferentiableObjective> DifferentiableObjectivePtr;

class StochasticObjective;
typedef ReferenceCountedObjectPtr<StochasticObjective> StochasticObjectivePtr;

class LearningObjective;
typedef ReferenceCountedObjectPtr<LearningObjective> LearningObjectivePtr;

class SupervisedLearningObjective;
typedef ReferenceCountedObjectPtr<SupervisedLearningObjective> SupervisedLearningObjectivePtr;

class SplittingCriterion;
typedef ReferenceCountedObjectPtr<SplittingCriterion> SplittingCriterionPtr;

class SelectionCriterion;
typedef ReferenceCountedObjectPtr<SelectionCriterion> SelectionCriterionPtr;

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

class CrowdingArchive;
typedef ReferenceCountedObjectPtr<CrowdingArchive> CrowdingArchivePtr;

class SolverEvaluator;
typedef ReferenceCountedObjectPtr<SolverEvaluator> SolverEvaluatorPtr;

class SolverCallback;
typedef ReferenceCountedObjectPtr<SolverCallback> SolverCallbackPtr;

class EvaluatorSolverCallback;
typedef ReferenceCountedObjectPtr<EvaluatorSolverCallback> EvaluatorSolverCallbackPtr;

class Solver;
typedef ReferenceCountedObjectPtr<Solver> SolverPtr;

class VariableEncoder;
typedef ReferenceCountedObjectPtr<VariableEncoder> VariableEncoderPtr;

class IterativeSolver;
typedef ReferenceCountedObjectPtr<IterativeSolver> IterativeSolverPtr;

class PopulationBasedSolver;
typedef ReferenceCountedObjectPtr<PopulationBasedSolver> PopulationBasedSolverPtr;

class Sampler;
typedef ReferenceCountedObjectPtr<Sampler> SamplerPtr;

/*
** DoubleVector
*/
class DoubleVector;
typedef ReferenceCountedObjectPtr<DoubleVector> DoubleVectorPtr;

class SparseDoubleVector;
typedef ReferenceCountedObjectPtr<SparseDoubleVector> SparseDoubleVectorPtr;

class DenseDoubleVector;
typedef ReferenceCountedObjectPtr<DenseDoubleVector> DenseDoubleVectorPtr;


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
class DataVector;
typedef ReferenceCountedObjectPtr<DataVector> DataVectorPtr;

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

class IncrementalLearner;
typedef ReferenceCountedObjectPtr<IncrementalLearner> IncrementalLearnerPtr;

}; /* namespace lbcpp */

#endif // !ML_PREDECLARATIONS_H_
