/*-----------------------------------------.---------------------------------.
| Filename: EnsembleLearner.h              | Ensemble Learner                |
| Author  : Francis Maes                   |                                 |
| Started : 02/01/2012 17:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_LEARNER_ENSEMBLE_H_
# define LBCPP_ML_LEARNER_ENSEMBLE_H_

# include <ml/Solver.h>

namespace lbcpp
{

class EnsembleLearner : public Solver
{
public:
  EnsembleLearner(size_t ensembleSize = 0)
    : ensembleSize(ensembleSize) {}

  virtual ExpressionPtr makeExpression(ExecutionContext& context, size_t iter) = 0;

  virtual void runSolver(ExecutionContext& context)
  {
    // get training and validation objectives
    SupervisedLearningObjectivePtr objective = problem->getObjective(0).staticCast<SupervisedLearningObjective>();
    SupervisedLearningObjectivePtr validationObjective;
    if (problem->getNumValidationObjectives())
      validationObjective = problem->getValidationObjective(0).staticCast<SupervisedLearningObjective>();
    
    // create aggregator and aggregator expressions
    ClassPtr supervisionType = objective->getSupervision()->getType();
    std::pair<AggregatorPtr, ClassPtr> aggregatorAndOutputType = createAggregator(supervisionType);
    AggregatorPtr aggregator = aggregatorAndOutputType.first;
    ClassPtr outputType = aggregatorAndOutputType.second;
    AggregatorExpressionPtr res = new AggregatorExpression(aggregator, outputType);
    res->reserveNodes(ensembleSize);
    
    // start aggregations for training and validation data
    ObjectPtr trainingData = aggregator->startAggregation(objective->getIndices(), outputType);
    ObjectPtr validationData;
    if (validationObjective && verbosity >= verbosityDetailed)
      validationData = aggregator->startAggregation(validationObjective->getIndices(), outputType);
    
    // build ensemble
    for (size_t i = 0; i < ensembleSize; ++i)
    {
      if (verbosity >= verbosityDetailed)
      {
        context.enterScope(T("Iteration ") + string((int)i));
        context.resultCallback(T("iteration"), i);
      }
      
      ExpressionPtr expression = makeExpression(context, i);
      if (expression)
      {
        res->pushNode(expression);
        aggregator->updateAggregation(trainingData, expression->compute(context, objective->getData(), objective->getIndices()));
        if (validationData)
          aggregator->updateAggregation(validationData, expression->compute(context, validationObjective->getData(), validationObjective->getIndices()));
      }

      if (verbosity >= verbosityDetailed)
      {
        DataVectorPtr trainingPredictions = aggregator->finalizeAggregation(trainingData);
        double trainingScore = objective->evaluatePredictions(context, trainingPredictions);
        context.resultCallback("trainingScore", trainingScore);
        if (validationObjective)
        {
          DataVectorPtr validationPredictions = aggregator->finalizeAggregation(validationData);
          double validationScore = validationObjective->evaluatePredictions(context, validationPredictions);
          context.resultCallback("validationScore", validationScore);
        }
        context.leaveScope();
      }
      if (verbosity >= verbosityProgressAndResult)
        context.progressCallback(new ProgressionState(i + 1, ensembleSize, T(" base models")));
    }

    // set the solution
    DataVectorPtr trainingPredictions = aggregator->finalizeAggregation(trainingData);
    double trainingScore = objective->evaluatePredictions(context, trainingPredictions);
    addSolution(context, res, new Fitness(trainingScore, problem->getFitnessLimits()));
  }

protected:
  friend class EnsembleLearnerClass;

  size_t ensembleSize;

  std::pair<AggregatorPtr, ClassPtr> createAggregator(ClassPtr supervisionType)
  {
    if (supervisionType->inheritsFrom(doubleClass))
      return std::make_pair(meanDoubleAggregator(), supervisionType);
    else if (supervisionType.isInstanceOf<Enumeration>())
      return std::make_pair(meanDoubleVectorAggregator(), denseDoubleVectorClass(supervisionType.staticCast<Enumeration>(), doubleClass));
    else
    {
      jassertfalse; // not implemented yet
      return std::make_pair(AggregatorPtr(), ClassPtr());
    }
  }
};

class SimpleEnsembleLearner : public EnsembleLearner
{
public:
  SimpleEnsembleLearner(const SolverPtr& baseLearner, size_t ensembleSize)
    : EnsembleLearner(ensembleSize), baseLearner(baseLearner) {}
  SimpleEnsembleLearner() {}

  virtual void runSolver(ExecutionContext& context)
  {
    baseLearner->startBatch(context);
    EnsembleLearner::runSolver(context);
    baseLearner->stopBatch(context);
  }

  virtual ExpressionPtr makeExpression(ExecutionContext& context, size_t iter)
  {
    ExpressionPtr expression;
    baseLearner->solve(context, problem, storeBestSolutionSolverCallback(*(ObjectPtr* )&expression));
    return expression;
  }

protected:
  friend class SimpleEnsembleLearnerClass;
  
  SolverPtr baseLearner;
};

class BaggingLearner : public SimpleEnsembleLearner
{
public:
  BaggingLearner(const SolverPtr& baseLearner, size_t ensembleSize)
    : SimpleEnsembleLearner(baseLearner, ensembleSize) {}
  BaggingLearner() {}
  
  virtual ExpressionPtr makeExpression(ExecutionContext& context, size_t iter)
  {
    SupervisedLearningObjectivePtr objective = problem->getObjective(0).staticCast<SupervisedLearningObjective>();
    IndexSetPtr oldIndices = objective->getIndices();
    objective->setIndices(objective->getIndices()->sampleBootStrap(context.getRandomGenerator()));
    ExpressionPtr res = SimpleEnsembleLearner::makeExpression(context, iter);
    objective->setIndices(oldIndices);
    return res;
  }
};

}; /* namespace lbcpp */

#endif // LBCPP_ML_LEARNER_ENSEMBLE_H_
