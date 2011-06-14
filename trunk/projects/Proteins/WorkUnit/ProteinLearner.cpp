/*-----------------------------------------.---------------------------------.
| Filename: ProteinLearner.cpp             | Protein Learner Work Unit       |
| Author  : Francis Maes                   |                                 |
| Started : 28/02/2011 16:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "ProteinLearner.h"
#include "../Evaluator/ProteinEvaluator.h"
using namespace lbcpp;

ProteinLearner::ProteinLearner() : maxProteins(0), numStacks(1), sequentialLearning(false)
{
  parameters = numericalProteinPredictorParameters();
  proteinTargets.push_back(ss3Target);
  proteinTargets.push_back(ss8Target);
  proteinTargets.push_back(sa20Target);
  proteinTargets.push_back(drTarget);
}

Variable ProteinLearner::run(ExecutionContext& context)
{
  if (!parameters)
  {
    context.errorCallback(T("No predictor parameters"));
    return false;
  }
  context.resultCallback(T("parameters"), parameters);
  
  // create predictor
  FunctionPtr predictor = createPredictor(context, parameters);
  if (!predictor)
    return false;
  
  // load proteins
  if (!supervisionDirectory.exists() || !supervisionDirectory.isDirectory())
  {
    context.errorCallback(T("Invalid supervision directory"));
    return false;
  }

  ContainerPtr trainProteins = loadProteinPairs(context, T("train"));
  ContainerPtr validationProteins = loadProteinPairs(context, T("validation"));
  if (!trainProteins || !validationProteins)
    return false;

  context.informationCallback(String((int)trainProteins->getNumElements()) + T(" training proteins, ") +
                              String((int)validationProteins->getNumElements()) + T(" validation proteins"));

  // train
  if (!predictor->train(context, trainProteins, validationProteins, T("Training")))
    return false;


  // evaluate
  EvaluatorPtr trainEvaluator = new ProteinEvaluator(true);
  ScoreObjectPtr trainScore = selectScoresFromTargets(trainEvaluator, predictor->evaluate(context, trainProteins, trainEvaluator, T("Evaluate on train proteins")));
  EvaluatorPtr validationEvaluator = new ProteinEvaluator(true);
  ScoreObjectPtr validationScore = selectScoresFromTargets(validationEvaluator, predictor->evaluate(context, validationProteins, validationEvaluator, T("Evaluate on validation proteins")));
  if (!trainScore || !validationScore)
    return false;

  // save predictions to directory
  if (predictionDirectory != File::nonexistent)
  {
    savePredictionsToDirectory(context, predictor, trainProteins, predictionDirectory.getChildFile(T("train")));
    savePredictionsToDirectory(context, predictor, validationProteins, predictionDirectory.getChildFile(T("validation")));
  }
  trainProteins = ContainerPtr();
  validationProteins = ContainerPtr();

  // evaluate/save predictions on test proteins
  ContainerPtr testProteins = loadProteinPairs(context, T("test"));
  ScoreObjectPtr testScore;
  if (testProteins)
  {
    context.informationCallback(String((int)testProteins->getNumElements()) + T(" test proteins"));
    EvaluatorPtr testEvaluator = new ProteinEvaluator(true);
    testScore = selectScoresFromTargets(testEvaluator, predictor->evaluate(context, testProteins, testEvaluator, T("Evaluate on test proteins")));
    if (!testScore)
      return false;

    if (predictionDirectory != File::nonexistent)
      savePredictionsToDirectory(context, predictor, testProteins, predictionDirectory.getChildFile(T("test")));
  }

  if (learnedModelFile != File::nonexistent)
  {
    context.informationCallback(T("Saving model to file ") + learnedModelFile.getFileName());
    predictor->saveToFile(context, learnedModelFile);
  }
  
  //size_t numFeaturesPerResidue = parameters->createResidueVectorPerception()->getOutputType()->getNumMemberVariables();
  return new ProteinLearnerScoreObject(trainScore, validationScore, testScore, 0);//, numFeaturesPerResidue);
}

ContainerPtr ProteinLearner::loadProteinPairs(ExecutionContext& context, const String& name) const
{
  File input = inputDirectory.exists() ? inputDirectory.getChildFile(name) : File::nonexistent;
  File supervision = supervisionDirectory.getChildFile(name);
  return Protein::loadProteinsFromDirectoryPair(context, input, supervision, maxProteins, T("Loading ") + name + T(" proteins"));
}

bool ProteinLearner::savePredictionsToDirectory(ExecutionContext& context, FunctionPtr predictor, ContainerPtr proteinPairs, const File& predictionDirectory) const
{
  return predictor->evaluate(context, proteinPairs, saveToDirectoryEvaluator(predictionDirectory, T(".xml")),
    T("Saving predictions to directory ") + predictionDirectory.getFileName());
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
  res->setEvaluator(new ProteinEvaluator());
  return res;
}

FunctionPtr ProteinLearner::createPredictor(ExecutionContext& context, ProteinPredictorParametersPtr parameters) const
{
  if (!numStacks)
  {
    context.errorCallback(T("No stacks. You need at least one stack"));
    return FunctionPtr();
  }
  
  if (sequentialLearning)
    return createSequentialPredictor(context, parameters);
  return createParallelPredictor(context, parameters);
}

FunctionPtr ProteinLearner::createParallelPredictor(ExecutionContext& context, ProteinPredictorParametersPtr parameters) const
{
  if (numStacks == 1)
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

FunctionPtr ProteinLearner::createSequentialPredictor(ExecutionContext& context, ProteinPredictorParametersPtr parameters) const
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

  ProteinSequentialPredictorPtr res = new ProteinSequentialPredictor();
  for (size_t i = 0; i < numStacks; ++i)
  {
    for (size_t i = 0; i < proteinTargets.size(); ++i)
    {
      ProteinPredictorPtr stack = new ProteinPredictor(parameters);
      stack->addTarget(proteinTargets[i]);
      stack->setEvaluator(new ProteinEvaluator());
      res->addPredictor(stack);
    }
  }

  return res;
}

ScoreObjectPtr ProteinLearner::selectScoresFromTargets(EvaluatorPtr evaluator, ScoreObjectPtr scores) const
{
  ReferenceCountedObjectPtr<ProteinEvaluator> proteinEvaluator = evaluator.dynamicCast<ProteinEvaluator>();
  if (!proteinEvaluator || !scores)
    return ScoreObjectPtr();

  ReferenceCountedObjectPtr<ProteinScoreObject> res = new ProteinScoreObject();
  for (size_t i = 0; i < proteinTargets.size(); ++i)
  {
    ScoreObjectPtr score = proteinEvaluator->getScoreObjectOfTarget(scores, proteinTargets[i]);
    if (score)
      res->addScoreObject(score);
  }
  return res;
}
