/*-----------------------------------------.---------------------------------.
| Filename: GPSandBox.h                    | Genetic Programming SandBox     |
| Author  : Francis Maes                   |                                 |
| Started : 22/05/2011 19:58               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_GENETIC_PROGRAMMING_SAND_BOX_H_
# define LBCPP_GENETIC_PROGRAMMING_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/Optimizer/OptimizerContext.h>
# include <lbcpp/Optimizer/OptimizerState.h>
# include "../GP/GPExpression.h"
# include "../GP/GPExpressionSampler.h"
# include "../GP/GPExpressionBuilder.h"
# include "../Core/SearchTree.h"
# include "../Core/SearchPolicy.h"
# include "../Core/SearchHeuristic.h"

namespace lbcpp
{

// GPExpression -> Double
class GPObjectiveFunction : public SimpleUnaryFunction
{
public:
  GPObjectiveFunction(const std::vector<std::pair< std::vector<double> , double> >& examples, double lambda)
    : SimpleUnaryFunction(gpExpressionClass, doubleType), lambda(lambda), examples(examples) {}
 
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const GPExpressionPtr& expression = input.getObjectAndCast<GPExpression>();

    double res = 0.0;
    for (size_t i = 0; i < examples.size(); ++i)
    {
      double prediction = expression->compute(&examples[i].first[0]);
      res += fabs(prediction - examples[i].second);
    }

    // tmp
    //context.informationCallback(expression->toShortString() + T(" -> ") + String(res / (double)examples.size()));
    // -

    return res / (double)examples.size() + lambda * expression->size();
  }
 
protected:
  double lambda;
  std::vector<std::pair<std::vector<double> , double> > examples;
};

// GPExpression -> Double
class GPStructureObjectiveFunction : public SimpleUnaryFunction
{
public:
  GPStructureObjectiveFunction(FunctionPtr objectiveFunction)
    : SimpleUnaryFunction(gpExpressionClass, doubleType), objectiveFunction(objectiveFunction)
  {
    objectiveFunction->initialize(defaultExecutionContext(), gpExpressionClass);
  }

  void getLearnableConstants(const GPExpressionPtr& expression, std::vector<double>& res) const
  {
    ConstantGPExpressionPtr constantExpression = expression.dynamicCast<ConstantGPExpression>();
    if (constantExpression)
    {
      if (constantExpression->isLearnable())
        res.push_back(constantExpression->getValue());
    }
    else if (expression.dynamicCast<BinaryGPExpression>())
    {
      const BinaryGPExpressionPtr& expr = expression.staticCast<BinaryGPExpression>();
      getLearnableConstants(expr->getLeft(), res);
      getLearnableConstants(expr->getRight(), res);
    }
    else if (expression.dynamicCast<UnaryGPExpression>())
    {
      const UnaryGPExpressionPtr& expr = expression.staticCast<UnaryGPExpression>();
      getLearnableConstants(expr->getExpression(), res);
    }
  }


  // DenseDoubleVector -> Double
  struct Objective : public SimpleUnaryFunction
  {
    Objective(FunctionPtr finalObjective, const GPExpressionPtr& structure)
      : SimpleUnaryFunction(denseDoubleVectorClass(), doubleType),
        finalObjective(finalObjective), structure(structure)
    {
    }

    void setConstantsRecursively(const GPExpressionPtr& expression, const DenseDoubleVectorPtr& constants, size_t& index) const
    {
      ConstantGPExpressionPtr constantExpression = expression.dynamicCast<ConstantGPExpression>();
      if (constantExpression)
      {
        if (constantExpression->isLearnable())
        {
          constantExpression->setValue(constants->getValue(index));
          ++index;
        }
      }
      else if (expression.dynamicCast<BinaryGPExpression>())
      {
        const BinaryGPExpressionPtr& expr = expression.staticCast<BinaryGPExpression>();
        setConstantsRecursively(expr->getLeft(), constants, index);
        setConstantsRecursively(expr->getRight(), constants, index);
      }
      else if (expression.dynamicCast<UnaryGPExpression>())
      {
        const UnaryGPExpressionPtr& expr = expression.staticCast<UnaryGPExpression>();
        setConstantsRecursively(expr->getExpression(), constants, index);
      }
    }

    virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {
      DenseDoubleVectorPtr constants = input.getObjectAndCast<DenseDoubleVector>();
      GPExpressionPtr expression = structure->cloneAndCast<GPExpression>();
      size_t index = 0;
      setConstantsRecursively(expression, constants, index);
      Variable res = finalObjective->compute(context, expression);
      return res.toDouble();
    }

  protected:
    FunctionPtr finalObjective;
    GPExpressionPtr structure;
  };

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    const GPExpressionPtr& expression = input.getObjectAndCast<GPExpression>();

    std::vector<double> constants;
    getLearnableConstants(expression, constants);
    if (constants.size() == 0)
      return new GPStructureScoreObject(expression->cloneAndCast<GPExpression>(), objectiveFunction->compute(context, input).toDouble());
    size_t numParameters = constants.size();

    //context.enterScope(T("Optimizing constants in ") + expression->toShortString());
    // Enumeration des constantes
    DefaultEnumerationPtr constantsEnumeration = new DefaultEnumeration(T("constants"));
    for (size_t i = 0; i < constants.size(); ++i)
      constantsEnumeration->addElement(context, T("cst") + String((int)i));

    // Sampler de DenseDoubleVector pour les constantes
    SamplerPtr constantsSampler = independentDoubleVectorSampler((EnumerationPtr)constantsEnumeration, gaussianSampler());

    // opmitizer context and state
    ReferenceCountedObjectPtr<Objective> objective = new Objective(objectiveFunction, expression);
    objective->initialize(context, denseDoubleVectorClass(constantsEnumeration));

    OptimizerContextPtr optimizerContext = synchroneousOptimizerContext(context, objective);
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(constantsSampler);

    // optimizer
    size_t numIterations = 100;
    size_t populationSize = numParameters * 12;
    size_t numBests = numParameters * 3;
    StoppingCriterionPtr stoppingCriterion = maxIterationsWithoutImprovementStoppingCriterion(5);
    
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests, stoppingCriterion, 0.0, false);
    ExecutionContextPtr silentContext = singleThreadedExecutionContext();
    optimizer->compute(*silentContext, optimizerContext, optimizerState);

    // retrieve best score and best constants and compute best expression
    double bestScore = optimizerState->getBestScore();
    DenseDoubleVectorPtr bestConstants = optimizerState->getBestVariable().getObjectAndCast<DenseDoubleVector>();
    GPExpressionPtr bestExpression = expression->cloneAndCast<GPExpression>();
    size_t index = 0;
    objective->setConstantsRecursively(bestExpression, bestConstants, index);
    return new GPStructureScoreObject(bestExpression, bestScore);
  }

protected:
  FunctionPtr objectiveFunction;
};

////////////////////////////////////////////////////////

class GPSandBox : public WorkUnit
{
public:
  GPSandBox() : lambda(0.01)
  {
    numIterations = 100;
    populationSize = 1000;
    numBests = 200;
  }

  virtual Variable run(ExecutionContext& context)
  {
    DefaultEnumerationPtr inputVariables = new DefaultEnumeration(T("variables"));
    inputVariables->addElement(context, T("x1"));
    inputVariables->addElement(context, T("x2"));
    inputVariables->addElement(context, T("x3"));
    inputVariables->addElement(context, T("x4"));

    FunctionPtr objective = createObjectiveFunction(context);
    if (!objective->initialize(context, gpExpressionClass))
      return false;

    return breadthFirstSearch(context, inputVariables, objective);

    //SamplerPtr sampler = createExpressionSampler(inputVariables);
    //for (size_t i = 0; i < 10; ++i)
    //  context.informationCallback(sampler->sample(context, random).toShortString());

    //return optimize(context, sampler, objective);
  }

  SamplerPtr createExpressionSampler(EnumerationPtr inputVariables) const
    {return new GPExpressionSampler(maximumEntropySampler(gpExprLabelsEnumeration), inputVariables, 1);}

  FunctionPtr createObjectiveFunction(ExecutionContext& context) const
  {
    RandomGeneratorPtr random = new RandomGenerator();
    std::vector<std::pair<std::vector<double> , double> > examples;
    for (size_t i = 0; i < 1000; ++i)
    {
      std::vector<double> x(4);

      x[0] = random->sampleDouble(1.0, 6.0);
      x[1] = random->sampleDouble(1.0, 6.0);
      x[2] = random->sampleDouble(1.0, 6.0);
      x[3] = random->sampleDouble(1.0, 6.0);
      double y = x[0] + log(x[1] * x[2]);// + x[2] + x[3];// 10.0 / (5.0 + (x[0] - 2) * (x[0] - 2) + (x[1] - 2) * (x[1] - 2));
//      double y = 1.0 + x[0] + x[1];//cos(x[0] * (1 + cos(x[0])));
      examples.push_back(std::make_pair(x, y));
    }

    return new GPObjectiveFunction(examples, lambda);
    //return new GPStructureObjectiveFunction(objective);
  }

  bool breadthFirstSearch(ExecutionContext& context, EnumerationPtr inputVariables, const FunctionPtr& objective)
  {
    size_t maxSearchNodes = 100;

    DecisionProblemPtr problem = new GPExpressionBuilderProblem(inputVariables, objective);
    DecisionProblemStatePtr state = new LargeGPExpressionBuilderState(T("toto"), inputVariables, objective);

    for (size_t depth = 0; depth < 10; ++depth)
    {
      context.enterScope(T("Depth ") + String((int)depth + 1));

      SearchTreePtr searchTree = new SearchTree(problem, state, maxSearchNodes);
      
      PolicyPtr searchPolicy = bestFirstSearchPolicy(new MinDepthSearchHeuristic());

      searchTree->doSearchEpisode(context, searchPolicy, maxSearchNodes);

      context.resultCallback(T("bestReturn"), searchTree->getBestReturn());
      bool isFinished = (searchTree->getBestReturn() <= 0.0);
      if (!isFinished)
      {
        context.resultCallback(T("bestAction"), searchTree->getBestAction());
        context.resultCallback(T("bestTrajectory"), searchTree->getBestNodeTrajectory());

        double transitionReward;
        state->performTransition(context, searchTree->getBestAction(), transitionReward);
        context.resultCallback(T("newState"), state->clone(context));
        context.resultCallback(T("transitionReward"), transitionReward);

        GPExpressionBuilderStatePtr expressionBuilderState = state.dynamicCast<LargeGPExpressionBuilderState>();
        jassert(expressionBuilderState);
        context.resultCallback(T("expression"), expressionBuilderState->getExpression());
        context.resultCallback(T("score"), expressionBuilderState->getScore());
        context.informationCallback(expressionBuilderState->getExpression()->toShortString());

        context.leaveScope(expressionBuilderState->getScore());
      }
      else
      {
        context.leaveScope(state);
        break;
      }
    }
    return true;
  }

  bool optimize(ExecutionContext& context, const SamplerPtr& sampler, const FunctionPtr& objective)
  {
    OptimizerContextPtr optimizerContext = multiThreadedOptimizerContext(context, objective);
    OptimizerStatePtr optimizerState = new SamplerBasedOptimizerState(sampler);

    // optimizer
    OptimizerPtr optimizer = edaOptimizer(numIterations, populationSize, numBests, StoppingCriterionPtr(), 0.0, true, false);
    optimizer->compute(context, optimizerContext, optimizerState);

    return true;
  }

protected:
  friend class GPSandBoxClass;

  double lambda;
  size_t numIterations;
  size_t populationSize;
  size_t numBests;
};


}; /* namespace lbcpp */

#endif // !LBCPP_GENETIC_PROGRAMMING_SAND_BOX_H_
