/*-----------------------------------------.---------------------------------.
| Filename: GraftingOnlineLearner.cpp      | Grafting Online Learner         |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2010 16:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <lbcpp/Data/RandomGenerator.h>
#include "GraftingOnlineLearner.h"
using namespace lbcpp;

GraftingOnlineLearner::GraftingOnlineLearner(PerceptionPtr perception, const std::vector<NumericalInferencePtr>& inferences)
  : learningStopped(false), inferences(inferences), perception(perception.staticCast<SelectAndMakeProductsPerception>())
{
  // create empty perception for candidates
  candidatesPerception = selectAndMakeProductsPerception(
                                this->perception->getDecoratedPerception(),
                                this->perception->getMultiplyFunction());

  // initialize scores mapping (inference -> first output index)
  size_t c = 0;
  for (size_t i = 0; i < inferences.size(); ++i)
  {
    size_t numOutputs = getNumOutputs(inferences[i]);
    jassert(numOutputs);
    if (numOutputs)
    {
      scoresMapping[inferences[i]] = c;
      c += numOutputs;
    }
  }
  jassert(c);

  // initialize candidate scores
  candidateScores.resize(c);
}

void GraftingOnlineLearner::startLearningCallback(ExecutionContext& context)
{
  learningStopped = false;
  generateCandidates(context, SortedConjunctions(), SortedConjunctions());
  resetCandidateScores();
  InferenceOnlineLearner::startLearningCallback(context);
}

void GraftingOnlineLearner::subStepFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  jassert(supervision.exists());
  std::map<NumericalInferencePtr, size_t>::const_iterator it = scoresMapping.find(inference);
  if (it != scoresMapping.end())
    updateCandidateScores(context, inference, it->second, input, supervision, prediction);
  InferenceOnlineLearner::subStepFinishedCallback(context, inference, input, supervision, prediction);
}

void GraftingOnlineLearner::stepFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  jassert(supervision.exists());
  std::map<NumericalInferencePtr, size_t>::const_iterator it = scoresMapping.find(inference);
  if (it != scoresMapping.end())
    updateCandidateScores(context, inference, it->second, input, supervision, prediction);
  InferenceOnlineLearner::stepFinishedCallback(context, inference, input, supervision, prediction);
}

void GraftingOnlineLearner::passFinishedCallback(ExecutionContext& context, const InferencePtr& inference, const InferenceBatchLearnerInputPtr& batchLearnerInput)
{
  // compute active feature scores
  std::vector<double> activeScores;
  computeActiveScores(context, activeScores);
  SortedConjunctions sortedActiveScores;
  for (size_t i = 0; i < activeScores.size(); ++i)
    sortedActiveScores.insert(std::make_pair(activeScores[i], std::make_pair(i, perception->getConjunction(i))));

  // compute candidate scores
  std::vector<double> candidateScores;
  Conjunction bestCandidate;
  double bestCandidateScore;
  computeCandidateScores(context, candidateScores, bestCandidate, bestCandidateScore);
  SortedConjunctions sortedCandidateScores;
  for (size_t i = 0; i < candidateScores.size(); ++i)
    sortedCandidateScores.insert(std::make_pair(candidateScores[i], std::make_pair(i, candidatesPerception->getConjunction(i))));

  // prune parameters
  pruneParameters(context, sortedActiveScores);

  // accept candidate and re-generate candidates set
  if (!acceptCandidates(context, bestCandidate, bestCandidateScore, sortedCandidateScores))
  {
    context.informationCallback(T("Grafting"), T("Finished!"));
    learningStopped = true;
    return;
  }
  generateCandidates(context, sortedActiveScores, sortedCandidateScores);
  resetCandidateScores();

  // update parameters type
  for (size_t i = 0; i < inferences.size(); ++i)
    inferences[i]->updateParametersType(context);

  // display some informations
  context.informationCallback(String::empty);
  context.informationCallback(T("Grafting"), T("=== ") + String((int)perception->getNumConjunctions()) + T(" active, ")
    + String((int)candidatesPerception->getNumConjunctions()) + T(" candidates ==="));
  InferenceOnlineLearner::passFinishedCallback(context, inference, batchLearnerInput);
}


template<class Type>
static inline void makeSetFromVector(std::set<Type>& res, const std::vector<Type>& source)
{
  for (size_t i = 0; i < source.size(); ++i)
    res.insert(source[i]);
}

void GraftingOnlineLearner::generateCandidates(ExecutionContext& context, const SortedConjunctions& activeScores, const SortedConjunctions& candidateScores)
{
  std::set<Conjunction> conjunctions;
  makeSetFromVector(conjunctions, perception->getConjunctions());

  candidatesPerception->clearConjunctions();

  // add each inactive output variable
  size_t n = perception->getDecoratedPerception()->getNumOutputVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Conjunction conjunction(1, i);
    if (conjunctions.find(conjunction) == conjunctions.end())
      candidatesPerception->addConjunction(conjunction);
  }

  size_t numActives = 10;
  size_t numCandidates = 10;
  SortedConjunctions::const_reverse_iterator iti, itj;
  size_t i, j;
  for (i = 0, iti = activeScores.rbegin(); i < numActives && iti != activeScores.rend(); ++i, ++iti)
  {
    const Conjunction& c1 = iti->second.second;
    for (j = 0, itj = candidateScores.rbegin(); j < numCandidates && itj != candidateScores.rend(); ++j, ++itj)
    {
      const Conjunction& c2 = itj->second.second;
      if (c1.size() == 1 && c2.size() == 1)
      {
        Conjunction conjunction = c1;
        conjunction.push_back(c2[0]);
        if (conjunctions.find(conjunction) == conjunctions.end())
        {
          conjunctions.insert(conjunction);
          candidatesPerception->addConjunction(conjunction);
        }
      }
    }
  }

  // we want at least 100 candidates, fill with randomly sampled candidates
  static const size_t wantedCount = 1000;
  RandomGeneratorPtr random = RandomGenerator::getInstance();
  for (size_t i = candidatesPerception->getNumConjunctions(); i < wantedCount; )
  {
    Conjunction conjunction(2);
    conjunction[0] = random->sampleSize(n);
    conjunction[1] = random->sampleSize(n);
    if (conjunctions.find(conjunction) == conjunctions.end())
    {
      conjunctions.insert(conjunction);
      candidatesPerception->addConjunction(conjunction);
      ++i;
    }
  }
}

String GraftingOnlineLearner::conjunctionToString(const Conjunction& conjunction) const
{
  String res;
  for (size_t i = 0; i < conjunction.size(); ++i)
  {
    res += perception->getDecoratedPerception()->getOutputVariableName(conjunction[i]);
    if (i < conjunction.size() - 1)
      res += T(" x ");
  }
  return res;
}

bool GraftingOnlineLearner::acceptCandidates(ExecutionContext& context, const Conjunction& bestCandidate, double bestCandidateScore, const SortedConjunctions& sortedScores)
{
  static const double threshold = 0.0001;

  // generate top-five
  size_t i = 0;
  bool res = false;
  for (SortedConjunctions::const_reverse_iterator it = sortedScores.rbegin(); i < 5 && it != sortedScores.rend(); ++i, ++it)
  {
    if (it->first < threshold)
      break;
    context.informationCallback(T("Grafting"), T("Incorporating ") + conjunctionToString(it->second.second) + T(" (") + String(it->first) + T(")"));
    perception->addConjunction(it->second.second);
    res = true;
  }

  if (res)
    return true;

  String info = T("At the end: ") + String((int)sortedScores.size()) + T(" sorted scores");
  if (sortedScores.size())
    info += T(", range: ") + String(sortedScores.begin()->first) + T(" .. ") + String(sortedScores.rbegin()->first);
  context.informationCallback(T("Grafting"), info);
  return false;
}

void GraftingOnlineLearner::pruneParameters(ExecutionContext& context, const SortedConjunctions& sortedActiveScores)
{
  static const double threshold = 0.001;
  size_t i = 0;
  std::set<size_t> conjunctionsToRemove;
  for (SortedConjunctions::const_iterator it = sortedActiveScores.begin(); it != sortedActiveScores.end() && it->first <= threshold; ++i, ++it)
  {
    conjunctionsToRemove.insert(it->second.first);
    context.informationCallback(T("Grafting"), T("Removing ") + String((int)i + 1)
      + T(": ") + conjunctionToString(it->second.second) + T(" (") + String(it->first) + T(")"));
  }
  if (conjunctionsToRemove.size())
    perception->removeConjunctions(conjunctionsToRemove);
}

size_t GraftingOnlineLearner::getNumOutputs(const InferencePtr& inference) const
{
  TypePtr outputType = inference->getOutputType(inference->getInputType());
  if (outputType->inheritsFrom(doubleType))
    return outputType;
  else
    return outputType->getNumMemberVariables();
}

void GraftingOnlineLearner::computeActiveScores(ExecutionContext& context, std::vector<double>& res) const
{
  size_t numActives = perception->getNumConjunctions();
  res.clear();
  res.resize(numActives, 0.0);

  //size_t outputIndex = 0;
  for (size_t i = 0; i < inferences.size(); ++i)
  {
    const NumericalInferencePtr& inference = inferences[i];
    size_t numOutputs = getNumOutputs(inference);
    jassert(numOutputs);
    for (size_t j = 0; j < numOutputs; ++j)
    {
      ObjectPtr weights = inference->getWeights();
      if (weights && numOutputs > 1)
        weights = weights->getVariable(j).getObject();
      if (weights)
      {
        for (size_t k = 0; k < numActives; ++k)
        {
          Variable subWeights = weights->getVariable(k);
          if (subWeights.exists())
          {
            double value = subWeights.isDouble() ? subWeights.getDouble() : lbcpp::l1norm(context, subWeights.getObject());
            if (value > res[k])
              res[k] = value;
          }
        }
      }
    }
  }
}

void GraftingOnlineLearner::resetCandidateScores()
{
  for (size_t i = 0; i < candidateScores.size(); ++i)
    candidateScores[i] = std::make_pair(ObjectPtr(), 0);
}

void GraftingOnlineLearner::computeCandidateScores(ExecutionContext& context, std::vector<double>& res, Conjunction& bestCandidate, double& bestCandidateScore) const
{
  size_t numCandidates = candidatesPerception->getNumConjunctions();
  size_t numScores = candidateScores.size();

  res.clear();
  res.resize(numCandidates, 0.0);
  bestCandidateScore = 0.0;
  bestCandidate.clear();
  for (size_t i = 0; i < numCandidates; ++i)
  {
    double scoresMax = 0.0;
    for (size_t j = 0; j < numScores; ++j)
    {
      double score = getCandidateScore(context, i, j);
      if (score > scoresMax)
        scoresMax = score;
    }
    res[i] = scoresMax;
    if (scoresMax > bestCandidateScore)
    {
      bestCandidateScore = scoresMax;
      bestCandidate = candidatesPerception->getConjunction(i);
    }
  }
}

double GraftingOnlineLearner::getCandidateScore(ExecutionContext& context, size_t candidateNumber, size_t scoreNumber) const
{
  const ObjectPtr& scores = candidateScores[scoreNumber].first;
  size_t examplesCount = candidateScores[scoreNumber].second;
  if (!scores)
    return 0.0;
  jassert(examplesCount);
  double invC = 1.0 / (double)examplesCount;

  jassert(scores->getNumVariables() == candidatesPerception->getNumConjunctions());
  Variable candidateScore = scores->getVariable(candidateNumber);
  if (candidateScore.isDouble())
    return fabs(candidateScore.getDouble()) * invC; // score of a single feature
  else
  {
    // score of a group of features
    jassert(candidateScore.isObject());
    return lbcpp::l1norm(context, candidateScore.getObject()) * invC;
  }
}

void GraftingOnlineLearner::updateCandidateScores(ExecutionContext& context, const NumericalInferencePtr& numericalInference, size_t firstScoreIndex, const Variable& input, const Variable& supervision, const Variable& pred)
{
  jassert(perception == numericalInference->getPerception());
  jassert(candidatesPerception->getNumConjunctions());

  Variable prediction = pred;
  if (prediction.isNil())
    prediction = numericalInference->computeFunction(context, input);

  if (numericalInference.dynamicCast<LinearInference>())
  {
    const ScalarFunctionPtr& loss = supervision.getObjectAndCast<ScalarFunction>(context);
    double derivative = loss->computeDerivative(prediction.getDouble());
    lbcpp::addWeighted(context, candidateScores[firstScoreIndex].first, candidatesPerception, input, derivative);
    ++candidateScores[firstScoreIndex].second;
  }
  else if (numericalInference.dynamicCast<MultiLinearInference>())
  {
    const MultiClassLossFunctionPtr& loss = supervision.getObjectAndCast<MultiClassLossFunction>(context);
    ObjectPtr gradient;
    jassert(prediction.isObject());
    loss->compute(context, prediction.getObject(), NULL, &gradient, 1.0);

    size_t n = gradient->getNumVariables();
    for (size_t i = 0; i < n; ++i)
    {
      double derivative = gradient->getVariable(i).getDouble();
      lbcpp::addWeighted(context, candidateScores[firstScoreIndex + i].first, candidatesPerception, input, derivative);
      ++candidateScores[firstScoreIndex + i].second;
    }
  }
  else
    jassert(false); // Unsupported. There is a design issue here, the interface of NumericalInference should be extended
}
