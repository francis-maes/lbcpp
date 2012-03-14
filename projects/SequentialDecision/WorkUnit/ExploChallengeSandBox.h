/*-----------------------------------------.---------------------------------.
| Filename: ExploChallengeSandBox.h        | Exploration Exploitation        |
| Author  : Francis Maes                   |  Challenge 3 SandBox            |
| Started : 13/03/2012 16:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_EXPLO_CHALLENGE_SANDBOX_H_
# define LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_EXPLO_CHALLENGE_SANDBOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/Optimizer.h>
# include "../GP/ExploChallengeFormulaSearchProblem.h"
# include "../../../src/Sampler/Composite/IndependentDoubleVectorSampler.h"

namespace lbcpp
{

class ExploChallengeReverseSampler : public IndependentDoubleVectorSampler
{
public:
  ExploChallengeReverseSampler()
  {
    elementsEnumeration = exploChallengeFormulaObjectiveParametersEnumeration;
    outputType = denseDoubleVectorClass(elementsEnumeration, doubleType);
    size_t n = elementsEnumeration->getNumElements();
    const double* params = ExploChallengeFormulaObjective::getInitialSamplerParameters();
    samplers.resize(n);
    for (size_t i = 0; i < n; ++i)
      samplers[i] = gaussianSampler(params[i*2], params[i*2+1]);
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    DenseDoubleVectorPtr res = IndependentDoubleVectorSampler::sample(context, random).getObjectAndCast<DenseDoubleVector>();
    ExploChallengeFormulaObjective::applyConstraints(res);
    return res;
  }
};

class ExploChallengeReverseObjective : public SimpleUnaryFunction
{
public:
  ExploChallengeReverseObjective(size_t surrogateHorizon, size_t surrogateNumDocuments)
    : SimpleUnaryFunction(denseDoubleVectorClass(exploChallengeFormulaObjectiveParametersEnumeration, doubleType), doubleType), surrogateHorizon(surrogateHorizon), surrogateNumDocuments(surrogateNumDocuments) {}

  FunctionPtr createFormulaObjective(const Variable& params) const
  {
    DenseDoubleVectorPtr parameters = params.getObjectAndCast<DenseDoubleVector>();
    return new ExploChallengeFormulaObjective(surrogateHorizon, surrogateNumDocuments, parameters);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    FunctionPtr formulaObjective = createFormulaObjective(input);
    double res = 0.0;
    for (size_t i = 0; i < scores.size(); ++i)
    {
      size_t numRuns = 1;
      double predictedScore = 0.0;
      for (size_t j = 0; j < numRuns; ++j)
        predictedScore += formulaObjective->compute(context, scores[i].first).toDouble();
      predictedScore /= numRuns;
      res = juce::jmax(res, fabs(scores[i].second - predictedScore));
    }
    return res;
  }

  void addReferenceScore(const GPExpressionPtr& formula, double score)
    {scores.push_back(std::make_pair(formula, score));}

  std::vector< std::pair<GPExpressionPtr, double> > scores;

  Variable validate(ExecutionContext& context, const Variable& solution) const
  {
    FunctionPtr formulaObjective = createFormulaObjective(solution);
    ScalarVariableStatisticsPtr stats = new ScalarVariableStatistics("errors");
    for (size_t i = 0; i < scores.size(); ++i)
    {
      GPExpressionPtr formula = scores[i].first;
      context.enterScope(formula->toShortString());
      context.resultCallback(T("formula number"), i);
      context.informationCallback(T("Target score: ") + String(scores[i].second));
      ScalarVariableMean mean;
      for (size_t j = 0; j < 100; ++j)
      {
        double predictedScore = formulaObjective->compute(context, formula).toDouble();
        mean.push(predictedScore);
        context.informationCallback(T("Predicted score: ") + String(predictedScore));
      }
      double delta = mean.getMean() - scores[i].second;
      stats->push(fabs(delta));
      context.leaveScope(delta);
    }
    return stats;
  }

private:
  size_t surrogateHorizon;
  size_t surrogateNumDocuments;
};

typedef ReferenceCountedObjectPtr<ExploChallengeReverseObjective> ExploChallengeReverseObjectivePtr;

class ExploChallengeSandBox : public WorkUnit
{
public:
  ExploChallengeSandBox() : surrogateHorizon(307000), surrogateNumDocuments(100)
  {
  }

  struct Policies
  {
    static GPExpressionPtr random()
      {return new ConstantGPExpression(1.0);}

    static GPExpressionPtr presentedCount()
      {return new VariableGPExpression(Variable(0, exploChallengeFormulaVariablesEnumeration));}

    static GPExpressionPtr rewardMean()
      {return new VariableGPExpression(Variable(1, exploChallengeFormulaVariablesEnumeration));}

    static GPExpressionPtr playedCount()
      {return new VariableGPExpression(Variable(2, exploChallengeFormulaVariablesEnumeration));}

    static GPExpressionPtr relativeIndex()
      {return new VariableGPExpression(Variable(3, exploChallengeFormulaVariablesEnumeration));}

    static GPExpressionPtr logPresentedCount()
      {return new UnaryGPExpression(gpLog, presentedCount());}

    static GPExpressionPtr ucb1(double C) // rk + sqrt(C log(t) / tk)
      {return new BinaryGPExpression(rewardMean(), gpAddition,
              new UnaryGPExpression(gpSquareRoot,
                new BinaryGPExpression(
                  new BinaryGPExpression(new ConstantGPExpression(C), gpMultiplication, logPresentedCount()),
                  gpDivision,
                  playedCount())));}

    static GPExpressionPtr minimalUCB(double C) // rk + C / tk
      {return new BinaryGPExpression(rewardMean(), gpAddition,
        new BinaryGPExpression(new ConstantGPExpression(C), gpDivision, playedCount()));}

    static GPExpressionPtr minimalUCB2(double C) // rk + C / sqrt(tk)
      {return new BinaryGPExpression(rewardMean(), gpAddition,
        new BinaryGPExpression(new ConstantGPExpression(C), gpDivision,
          new UnaryGPExpression(gpSquareRoot, playedCount())));}

    static GPExpressionPtr maxRkC(double C)  // max(rk, C)
      {return new BinaryGPExpression(rewardMean(), gpMax, new ConstantGPExpression(C));}

  };

  ExploChallengeReverseObjectivePtr createObjective() const
  {
    ExploChallengeReverseObjectivePtr objective = new ExploChallengeReverseObjective(surrogateHorizon, surrogateNumDocuments);

    VariableGPExpressionPtr presentedCount = new VariableGPExpression(Variable(0, exploChallengeFormulaVariablesEnumeration));
    VariableGPExpressionPtr rewardMean = new VariableGPExpression(Variable(1, exploChallengeFormulaVariablesEnumeration));
    VariableGPExpressionPtr playedCount = new VariableGPExpression(Variable(2, exploChallengeFormulaVariablesEnumeration));
    VariableGPExpressionPtr relativeIndex = new VariableGPExpression(Variable(3, exploChallengeFormulaVariablesEnumeration));

    // baselines
    objective->addReferenceScore(Policies::random(), 366.7);
    objective->addReferenceScore(Policies::rewardMean(), 711.1); // Greedy
    //objective->addReferenceScore(relativeIndex, 266.7); // Always first
    //objective->addReferenceScore(new UnaryGPExpression(gpOpposite, relativeIndex), 512.6); // Always last

    // ucb1
    objective->addReferenceScore(Policies::ucb1(0.01), 849.4);
    objective->addReferenceScore(Policies::ucb1(0.05), 838.8);
    objective->addReferenceScore(Policies::ucb1(0.1), 817.4);
    objective->addReferenceScore(Policies::ucb1(0.2), 762.5);

    // minimal ucb 1
    objective->addReferenceScore(Policies::minimalUCB(0.15), 776.8);
    objective->addReferenceScore(Policies::minimalUCB(1.0), 846.6);
    objective->addReferenceScore(Policies::minimalUCB(1.5), 876.3);
    objective->addReferenceScore(Policies::minimalUCB(2.0), 862.2);
    objective->addReferenceScore(Policies::minimalUCB(30.0), 723.6);
    objective->addReferenceScore(Policies::minimalUCB(50.0), 650.5);

    // minimal ucb 2
    objective->addReferenceScore(Policies::minimalUCB2(0.2), 864.8);
    objective->addReferenceScore(Policies::minimalUCB2(0.3), 877.2);
    objective->addReferenceScore(Policies::minimalUCB2(0.4), 866.3);
    objective->addReferenceScore(Policies::minimalUCB2(0.5), 859.9);

    // max(rk, C)
    objective->addReferenceScore(Policies::maxRkC(0.01), 670.8);
    objective->addReferenceScore(Policies::maxRkC(0.02), 747.5);
    objective->addReferenceScore(Policies::maxRkC(0.03), 740.1);
    objective->addReferenceScore(Policies::maxRkC(0.04), 763.1);
    objective->addReferenceScore(Policies::maxRkC(0.05), 768.7);
    objective->addReferenceScore(Policies::maxRkC(0.06), 757.1);
    objective->addReferenceScore(Policies::maxRkC(0.07), 761.1);
    
    // misc

    // Rk + 0.2 log(Tk) / T	==> 682.6
    objective->addReferenceScore(new BinaryGPExpression(Policies::rewardMean(), gpAddition,
      new BinaryGPExpression(new ConstantGPExpression(0.2), gpMultiplication,
        new BinaryGPExpression(new UnaryGPExpression(gpLog, Policies::playedCount()), gpDivision, Policies::presentedCount()))), 682.6);

    // log(Tk)(Rk - 0.2)	==> 767.0
    objective->addReferenceScore(new BinaryGPExpression(
      new UnaryGPExpression(gpLog, Policies::playedCount()),
      gpMultiplication,
      new BinaryGPExpression(Policies::rewardMean(), gpSubtraction, new ConstantGPExpression(0.2))), 767.0);
    return objective;
  }

  virtual Variable run(ExecutionContext& context)
  {
    ExploChallengeReverseObjectivePtr objective = createObjective();
    SamplerPtr sampler = new ExploChallengeReverseSampler();
    OptimizationProblemPtr problem = new OptimizationProblem(objective, Variable(), sampler);
    OptimizerPtr optimizer = edaOptimizer(10, 100, 30, StoppingCriterionPtr(), 0.0, false, true);
    OptimizerStatePtr state = optimizer->optimize(context, problem);


    context.informationCallback(T("Best solution found: ") + state->getBestSolution().toShortString());
    context.informationCallback(T("Best solution score: ") + String(state->getBestScore()));

    SamplerPtr finalSampler = state->getVariable(3).getObjectAndCast<Sampler>();
    Variable samplerExpectation = finalSampler->computeExpectation();
    context.informationCallback(T("Sampler expectation: ") + samplerExpectation.toShortString());

    //context.resultCallback("state", state);

    context.enterScope(T("Validate Best Solution"));
    Variable res = objective->validate(context, state->getBestSolution());
    context.leaveScope(res);

    context.enterScope(T("Validate Sampler Expectation"));
    res = objective->validate(context, samplerExpectation);
    context.leaveScope(res);
    return true;
  }

protected:
  friend class ExploChallengeSandBoxClass;

  size_t surrogateHorizon;
  size_t surrogateNumDocuments;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_EXPLO_CHALLENGE_SANDBOX_H_
