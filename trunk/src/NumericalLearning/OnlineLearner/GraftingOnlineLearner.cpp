/*-----------------------------------------.---------------------------------.
| Filename: GraftingOnlineLearner.cpp      | Grafting Online Learner         |
| Author  : Francis Maes                   |                                 |
| Started : 19/10/2010 16:19               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "GraftingOnlineLearner.h"
using namespace lbcpp;

GraftingOnlineLearner::GraftingOnlineLearner(PerceptionPtr perception, const std::vector<InferencePtr>& inferences)
  : ProxyOnlineLearner(inferences), learningStopped(false), perception(perception.checkCast<SelectAndMakeProductsPerception>(T("GraftingOnlineLearner")))
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

void GraftingOnlineLearner::startLearningCallback()
{
  learningStopped = false;
  generateCandidates();
  resetCandidateScores();
}

void GraftingOnlineLearner::subStepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  stepFinishedCallback(inference, input, supervision, prediction);
}

void GraftingOnlineLearner::stepFinishedCallback(const InferencePtr& inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  jassert(supervision.exists());
  std::map<InferencePtr, size_t>::const_iterator it = scoresMapping.find(inference);
  if (it != scoresMapping.end())
    updateCandidateScores(inference, it->second, input, supervision, prediction);
}

void GraftingOnlineLearner::passFinishedCallback(const InferencePtr& inference)
{
  acceptCandidates();
  pruneParameters();
  generateCandidates();
  resetCandidateScores();
  MessageCallback::info(String::empty);
  MessageCallback::info(T("Grafting"), T("=== ") + String((int)perception->getNumConjunctions()) + T(" active, ")
    + String((int)candidatesPerception->getNumConjunctions()) + T(" candidates ==="));
}


template<class Type>
static inline void makeSetFromVector(std::set<Type>& res, const std::vector<Type>& source)
{
  for (size_t i = 0; i < source.size(); ++i)
    res.insert(source[i]);
}

void GraftingOnlineLearner::generateCandidates()
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

void GraftingOnlineLearner::acceptCandidates()
{
  static const double regularizerWeight = 0.0001;

  // compute candidate scores
  std::vector<double> scores;
  Conjunction bestCandidate;
  double bestCandidateScore;
  computeCandidateScores(scores, bestCandidate, bestCandidateScore);

  // generate top-five
  std::multimap<double, String> sortedScores;
  for (size_t i = 0; i < scores.size(); ++i)
    sortedScores.insert(std::make_pair(scores[i], conjunctionToString(candidatesPerception->getConjunction(i))));
  size_t i = 0;
  for (std::multimap<double, String>::reverse_iterator it = sortedScores.rbegin(); i < 5 && it != sortedScores.rend(); ++i, ++it)
  {
    if (!it->first)
      break;
    MessageCallback::info(T("Grafting"), T("Top ") + String((int)i + 1) + T(": ") + it->second + T(" (") + String(it->first) + T(")"));
  }

  // select top candidate
  if (bestCandidateScore > regularizerWeight)
  {
    MessageCallback::info(T("Grafting"), T("Incorporating ") + conjunctionToString(bestCandidate));
    perception->addConjunction(bestCandidate);
  }
  else
  {
    MessageCallback::info(T("Grafting"), T("Finished!"));
    learningStopped = true;
  }
}

void GraftingOnlineLearner::pruneParameters()
{
  // todo
}

size_t GraftingOnlineLearner::getNumOutputs(const InferencePtr& inference) const
{
  TypePtr outputType = inference->getOutputType(inference->getInputType());
  if (outputType->inheritsFrom(doubleType))
    return outputType;
  else
    return outputType->getObjectNumVariables();
}

void GraftingOnlineLearner::resetCandidateScores()
{
  for (size_t i = 0; i < candidateScores.size(); ++i)
    candidateScores[i] = std::make_pair(ObjectPtr(), 0);
}

void GraftingOnlineLearner::computeCandidateScores(std::vector<double>& res, Conjunction& bestCandidate, double& bestCandidateScore) const
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
      double score = getCandidateScore(i, j);
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

double GraftingOnlineLearner::getCandidateScore(size_t candidateNumber, size_t scoreNumber) const
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
    return lbcpp::l1norm(candidateScore.getObject()) * invC;
  }
}

void GraftingOnlineLearner::updateCandidateScores(const NumericalInferencePtr& numericalInference, size_t firstScoreIndex, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  jassert(perception == numericalInference->getPerception());
  jassert(candidatesPerception->getNumConjunctions());

  if (numericalInference.dynamicCast<LinearInference>())
  {
    const ScalarFunctionPtr& loss = supervision.getObjectAndCast<ScalarFunction>();
    double derivative = loss->computeDerivative(prediction.getDouble());
    lbcpp::addWeighted(candidateScores[firstScoreIndex].first, candidatesPerception, input, derivative);
    ++candidateScores[firstScoreIndex].second;
  }
  else if (numericalInference.dynamicCast<MultiLinearInference>())
  {
    const MultiClassLossFunctionPtr& loss = supervision.getObjectAndCast<MultiClassLossFunction>();
    ObjectPtr gradient;
    jassert(prediction.isObject());
    loss->compute(prediction.getObject(), NULL, &gradient, 1.0);

    size_t n = gradient->getNumVariables();
    for (size_t i = 0; i < n; ++i)
    {
      double derivative = gradient->getVariable(i).getDouble();
      lbcpp::addWeighted(candidateScores[firstScoreIndex + i].first, candidatesPerception, input, derivative);
      ++candidateScores[firstScoreIndex + i].second;
    }
  }
  else
    jassert(false); // Unsupported. There is a design issue here, the interface of NumericalInference should be extended
}
