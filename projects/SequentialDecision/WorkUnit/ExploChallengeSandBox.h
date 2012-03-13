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

extern EnumerationPtr exploChallengeReverseObjectiveVariablesEnumeration;

class ExploChallengeReverseSampler : public IndependentDoubleVectorSampler
{
public:
  ExploChallengeReverseSampler()
    : IndependentDoubleVectorSampler(exploChallengeReverseObjectiveVariablesEnumeration, gaussianSampler(0.5, 0.5)) {}

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const
  {
    DenseDoubleVectorPtr res = IndependentDoubleVectorSampler::sample(context, random).getObjectAndCast<DenseDoubleVector>();
    res->setValue(0, juce::jlimit(0.0, 1.0, res->getValue(0)));
    res->setValue(1, juce::jlimit(res->getValue(0), 1.0, res->getValue(1)));
    res->setValue(2, juce::jlimit(0.0, 1.0, res->getValue(2)));
    res->setValue(3, juce::jlimit(0.0, res->getValue(2), res->getValue(3)));
    res->setValue(4, juce::jmax(0.0, res->getValue(4)));
    return res;
  }
};

class ExploChallengeReverseObjective : public SimpleUnaryFunction
{
public:
  ExploChallengeReverseObjective(size_t surrogateHorizon)
    : SimpleUnaryFunction(denseDoubleVectorClass(exploChallengeReverseObjectiveVariablesEnumeration, doubleType), doubleType), surrogateHorizon(surrogateHorizon) {}

  FunctionPtr createFormulaObjective(const Variable& params) const
  {
    DenseDoubleVectorPtr parameters = params.getObjectAndCast<DenseDoubleVector>();

    //size_t totalNumArms = 246;
    //size_t numArmsPerRound = 30;
    size_t totalNumArms = (size_t)(pow(10.0, parameters->getValue(4) + 1.0));
    size_t numDocumentsAliveSimultaneously = (size_t)(parameters->getValue(2) * totalNumArms); // 80
    size_t numArmsPerRound = (size_t)(parameters->getValue(3) * totalNumArms);
    double minRewardExpectation = juce::jlimit(0.0, 1.0, parameters->getValue(0));
    double maxRewardExpectation = juce::jlimit(0.0, 1.0, parameters->getValue(1));

    return new ExploChallengeFormulaObjective
      (surrogateHorizon, totalNumArms,  numArmsPerRound, numDocumentsAliveSimultaneously, minRewardExpectation, maxRewardExpectation);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    FunctionPtr formulaObjective = createFormulaObjective(input);
    double res = 0.0;
    for (size_t i = 0; i < scores.size(); ++i)
    {
      double predictedScore = formulaObjective->compute(context, scores[i].first).toDouble();
      res += fabs(scores[i].second - predictedScore);
    }
    return res / scores.size();
  }

  void addReferenceScore(const GPExpressionPtr& formula, double score)
    {scores.push_back(std::make_pair(formula, score));}

  std::vector< std::pair<GPExpressionPtr, double> > scores;

  double validate(ExecutionContext& context, const Variable& solution) const
  {
    FunctionPtr formulaObjective = createFormulaObjective(solution);
    double res = 0.0;
    for (size_t i = 0; i < scores.size(); ++i)
    {
      GPExpressionPtr formula = scores[i].first;
      context.enterScope(formula->toShortString());
      context.informationCallback(T("Target score: ") + String(scores[i].second));
      ScalarVariableMean mean;
      for (size_t j = 0; j < 100; ++j)
      {
        double predictedScore = formulaObjective->compute(context, formula).toDouble();
        mean.push(predictedScore);
        context.informationCallback(T("Predicted score: ") + String(predictedScore));
      }
      double delta = mean.getMean() - scores[i].second;
      res += fabs(delta);
      context.leaveScope(delta);
    }
    return res / scores.size();
  }

private:
  size_t surrogateHorizon;
};

typedef ReferenceCountedObjectPtr<ExploChallengeReverseObjective> ExploChallengeReverseObjectivePtr;

class ExploChallengeSandBox : public WorkUnit
{
public:
  ExploChallengeSandBox() : surrogateHorizon(307000)
  {


  }

  ExploChallengeReverseObjectivePtr createObjective() const
  {
    VariableGPExpressionPtr presentedCount = new VariableGPExpression(Variable(0, exploChallengeFormulaVariablesEnumeration));
    VariableGPExpressionPtr rewardMean = new VariableGPExpression(Variable(1, exploChallengeFormulaVariablesEnumeration));
    VariableGPExpressionPtr playedCount = new VariableGPExpression(Variable(2, exploChallengeFormulaVariablesEnumeration));
    VariableGPExpressionPtr relativeIndex = new VariableGPExpression(Variable(3, exploChallengeFormulaVariablesEnumeration));

    ExploChallengeReverseObjectivePtr objective = new ExploChallengeReverseObjective(surrogateHorizon);

    // baselines
    //objective->addReferenceScore(relativeIndex, 266.7); // Always first
    for (size_t i = 0; i < 3; ++i)
    {
      objective->addReferenceScore(new ConstantGPExpression(1.0), 366.7); // C(1)
    //objective->addReferenceScore(new UnaryGPExpression(gpOpposite, relativeIndex), 512.6); // Always last
   

    // minimalistic ucb
      objective->addReferenceScore(new BinaryGPExpression(rewardMean, gpAddition,
        new BinaryGPExpression(new ConstantGPExpression(2), gpDivision, playedCount)), 862.2);  // B(add, V(rk), B(div, C(2), V(tk)))
    }
    return objective;

    objective->addReferenceScore(new BinaryGPExpression(rewardMean, gpAddition,
      new BinaryGPExpression(new ConstantGPExpression(30), gpDivision, playedCount)), 723.6);
    objective->addReferenceScore(new BinaryGPExpression(rewardMean, gpAddition,
      new BinaryGPExpression(new ConstantGPExpression(50), gpDivision, playedCount)), 650.5);

    // max(rk, C)
    objective->addReferenceScore(new BinaryGPExpression(rewardMean, gpMax, new ConstantGPExpression(0.01)), 670.8);
    return objective;
  }

  virtual Variable run(ExecutionContext& context)
  {
    ExploChallengeReverseObjectivePtr objective = createObjective();
    SamplerPtr sampler = new ExploChallengeReverseSampler();
    OptimizationProblemPtr problem = new OptimizationProblem(objective, Variable(), sampler);
    OptimizerPtr optimizer = edaOptimizer(30, 100, 25, StoppingCriterionPtr(), 0.0, false, true);
    OptimizerStatePtr state = optimizer->optimize(context, problem);


    context.informationCallback(T("Best solution found: ") + state->getBestSolution().toShortString());
    context.informationCallback(T("Best solution score: ") + String(state->getBestScore()));

    SamplerPtr finalSampler = state->getVariable(3).getObjectAndCast<Sampler>();
    Variable samplerExpectation = finalSampler->computeExpectation();
    context.informationCallback(T("Sampler expectation: ") + samplerExpectation.toShortString());

    //context.resultCallback("state", state);

    context.enterScope(T("Validate Best Solution"));
    double score = objective->validate(context, state->getBestSolution());
    context.leaveScope(score);

    context.enterScope(T("Validate Sampler Expectation"));
    score = objective->validate(context, samplerExpectation);
    context.leaveScope(score);
    return true;
  }

protected:
  friend class ExploChallengeSandBoxClass;

  size_t surrogateHorizon;
};

}; /* namespace lbcpp */

#endif // !LBCPP_SEQUENTIAL_DECISION_WORK_UNIT_EXPLO_CHALLENGE_SANDBOX_H_
