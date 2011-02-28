/*-----------------------------------------.---------------------------------.
| Filename: ProteinLearner.cpp             | Protein Learner Work Unit       |
| Author  : Francis Maes                   |                                 |
| Started : 28/02/2011 16:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinLearner.h"
#include "../Evaluator/ProteinEvaluator.h"
using namespace lbcpp;


ProteinLearner::ProteinLearner()
  : maxProteins(0), numFolds(7), maxLearningIterations(100), numStacks(1)
{
  proteinTargets.push_back(ss3Target);
  proteinTargets.push_back(ss8Target);
  proteinTargets.push_back(sa20Target);
  proteinTargets.push_back(drTarget);
}

FunctionPtr ProteinLearner::createOneStackPredictor(ExecutionContext& context, ProteinPredictorParametersPtr parameters) const
{
  if (proteinTargets.empty())
  {
    context.errorCallback(T("No protein targets were selected"));
    return FunctionPtr();
  }
  std::set<ProteinTarget> uniqueTargets;
  for (size_t i = 0; i < proteinTargets.size(); ++i)
  {
    if (uniqueTargets.find(proteinTargets[i]) != uniqueTargets.end())
    {
      context.errorCallback(T("Protein targets are not unique: ") + proteinClass->getMemberVariableName(proteinTargets[i]) + T(" appears multiple times"));
      return FunctionPtr();
    }
    uniqueTargets.insert(proteinTargets[i]);
  }

  ProteinPredictorPtr res = new ProteinPredictor(parameters);
  for (size_t i = 0; i < proteinTargets.size(); ++i)
    res->addTarget(proteinTargets[i]);
  return res;
}

FunctionPtr ProteinLearner::createPredictor(ExecutionContext& context, ProteinPredictorParametersPtr parameters) const
{
  if (!numStacks)
  {
    context.errorCallback(T("No stacks. You need at least one stack"));
    return FunctionPtr();
  }
  else if (numStacks == 1)
    return createOneStackPredictor(context, parameters);
  else
  {
    ProteinSequentialPredictorPtr res = new ProteinSequentialPredictor();
    for (size_t i = 0; i < numStacks; ++i)
    {
      FunctionPtr stack = createOneStackPredictor(context, parameters);
      if (!stack)
        return FunctionPtr();
      res->addPredictor(stack);
    }
    return res;
  }
}

Variable ProteinLearner::run(ExecutionContext& context)
{
  // create predictor
  ProteinPredictorParametersPtr parameters = new MyProteinPredictorParameters(maxLearningIterations);
  FunctionPtr predictor = createPredictor(context, parameters);
  if (!predictor)
    return false;

  // load proteins
  if (!supervisionDirectory.exists() || !supervisionDirectory.isDirectory())
  {
    context.errorCallback(T("Invalid supervision directory"));
    return false;
  }
  ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, maxProteins, T("Loading"));
  if (!proteins)
    return false;

  // make train and test proteins
  ContainerPtr trainingProteins = proteins->invFold(0, numFolds);
  ContainerPtr testingProteins = proteins->fold(0, numFolds);
  context.informationCallback(String((int)trainingProteins->getNumElements()) + T(" training proteins, ") +
                             String((int)testingProteins->getNumElements()) + T(" testing proteins"));
  

  // train
  if (!predictor->train(context, trainingProteins, ContainerPtr(), T("Training"), true))
    return false;

  // evaluate on training data
  if (!predictor->evaluate(context, trainingProteins, new ProteinEvaluator(), T("Evaluate on training data")))
    return false;
  
  // evaluate on testing data
  if (!predictor->evaluate(context, testingProteins, new ProteinEvaluator(), T("Evaluate on testing data")))
    return false;

  return true;
}
