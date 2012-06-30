/*-----------------------------------------.---------------------------------.
| Filename: OnlineLearningMCAlgorithm.h    | Online Learning MCAlgorithm     |
| Author  : Francis Maes                   |                                 |
| Started : 30/06/2012 10:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_MC_ALGORITHM_ONLINE_LEARNING_H_
# define LBCPP_LUAPE_MC_ALGORITHM_ONLINE_LEARNING_H_

# include "MCAlgorithm.h"

namespace lbcpp
{

class OnlineLearningMCAlgorithm : public MCAlgorithm
{
public:
  OnlineLearningMCAlgorithm(double learningRate = 1.0)
    : learningRate(learningRate) {}

  virtual void startSearch(ExecutionContext& context, DecisionProblemStatePtr initialState)
  {
    stepNumber = 0;
  }

  struct Step
  {
    ObjectVectorPtr actionFeatures;
    DenseDoubleVectorPtr actionActivations;
    size_t selectedAction;
  };

  virtual void search(ExecutionContext& context, MCObjectivePtr objective, const std::vector<Variable>& previousActions, DecisionProblemStatePtr state)
  {
    std::vector<Step> trajectory;

    state = state->cloneAndCast<DecisionProblemState>();
    std::vector<Variable> actions(previousActions);
    while (!state->isFinalState())
    {
      if (objective->shouldStop())
        return;

      // enumerate actions and compute action features
      ContainerPtr availableActions = state->getAvailableActions();
      size_t n = availableActions->getNumElements();
      Step step;
      step.actionFeatures = state->computeActionFeatures(context, availableActions);
      jassert(n == step.actionFeatures->getNumElements());

      // compute action selection probabilities
      step.actionActivations = new DenseDoubleVector(n, 0.0);
      std::vector<double> probabilities(n);
      double probabilitiesSum = 0.0;
      for (size_t i = 0; i < n; ++i)
      {
        double activation = predictActivation(step.actionFeatures->getAndCast<DoubleVector>(i));
        step.actionActivations->setValue(i, activation);
        double p = exp(activation);
        probabilities[i] = p;
        probabilitiesSum += p;
      }

      // select action
      step.selectedAction = context.getRandomGenerator()->sampleWithProbabilities(probabilities, probabilitiesSum);
      Variable action = availableActions->getElement(step.selectedAction);

      // perform transition and store information
      double reward;
      state->performTransition(context, action, reward);
      actions.push_back(action);
      trajectory.push_back(step);
    }

    // submit solution
    double score = submitFinalState(context, objective, actions, state);

    // update parameters
    scoreStatistics.push(score);
    double stddev = scoreStatistics.getStandardDeviation();
    if (stddev > 0.0)
    {
      double normalizedScore = (score - scoreStatistics.getMean()) / stddev;
      makeSGDStep(context, trajectory, normalizedScore);
      if (stepNumber % 1 == 0)
      {
        context.enterScope("Step " + String((int)stepNumber));
        context.resultCallback("step", stepNumber);
        context.resultCallback("score", score);
        context.resultCallback("normalizedScore", normalizedScore);
        context.resultCallback("scoreMean", scoreStatistics.getMean());
        context.resultCallback("scoreStddev", scoreStatistics.getStandardDeviation());
        context.resultCallback("scoreMax", scoreStatistics.getMaximum());
        context.resultCallback("parametersL0Norm", parameters->l0norm());
        context.resultCallback("parametersL1Norm", parameters->l1norm());
        context.resultCallback("parametersL2Norm", parameters->l2norm());
        context.leaveScope();
      }
      ++stepNumber;
    }
  }

  virtual void finishSearch(ExecutionContext& context)
  {
    parameters = DenseDoubleVectorPtr();
    scoreStatistics.clear();
  }

protected:
  friend class OnlineLearningMCAlgorithmClass;

  double learningRate;

  ScalarVariableStatistics scoreStatistics;
  DenseDoubleVectorPtr parameters;
  size_t stepNumber;

  double predictActivation(const DoubleVectorPtr& features) const
    {return parameters ? features->dotProduct(parameters, 0) : 0.0;}

  void makeSGDStep(ExecutionContext& context, const std::vector<Step>& trajectory, double normalizedScore)
  {
    if (!parameters)
    {
      ClassPtr dvClass = denseDoubleVectorClass(DoubleVector::getElementsEnumeration(trajectory[0].actionFeatures->getElementsType()), doubleType);
      parameters = new DenseDoubleVector(dvClass);
    }

    for (size_t i = 0; i < trajectory.size(); ++i)
    {
      const Step& step = trajectory[i];
      double logZ = step.actionActivations->computeLogSumOfExponentials();
      jassert(isNumberValid(logZ));
      size_t n = step.actionActivations->getNumValues();

      DenseDoubleVectorPtr gradient = new DenseDoubleVector(n, 0.0); 
      for (size_t i = 0; i < n; ++i)
      {
        double score = step.actionActivations->getValue(i);
        double derivative = exp(score - logZ);
        jassert(isNumberValid(derivative));
        gradient->incrementValue(i, derivative);
      }
      gradient->decrementValue(step.selectedAction, 1.0);

      for (size_t i = 0; i < n; ++i)
      {
        DoubleVectorPtr features = step.actionFeatures->getAndCast<DoubleVector>(i);
        features->addWeightedTo(parameters, 0, -learningRate * normalizedScore * gradient->getValue(i));
      }
    }
  }
};


}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_MC_ALGORITHM_ONLINE_LEARNING_H_
