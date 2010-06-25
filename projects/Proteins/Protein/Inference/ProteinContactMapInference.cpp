/*-----------------------------------------.---------------------------------.
| Filename: ProteinContactMapInference.cpp | Protein Contact Map Inference   |
| Author  : Francis Maes                   |                                 |
| Started : 23/06/2010 15:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "ProteinContactMapInference.h"
using namespace lbcpp;

/*
** ContactMapScoresInference
*/
ContactMapScoresInference::ContactMapScoresInference(const String& name, InferencePtr scoreInference, ProteinResiduePairFeaturesPtr features, const String& targetName)
  : Protein2DTargetInference(name, scoreInference, features, targetName)
  {}

void ContactMapScoresInference::computeSubStepIndices(ProteinPtr protein, std::vector< std::pair<size_t, size_t> >& res) const
{
  size_t n = protein->getLength();
  res.reserve(n * (n - 5) / 2);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i + 6; j < n; ++j)
      res.push_back(std::make_pair(i, j));
}

ObjectPtr ContactMapScoresInference::getSubSupervision(ObjectPtr supervisionObject, size_t firstPosition, size_t secondPosition) const
{
  if (!supervisionObject)
    return ObjectPtr();
  ScoreSymmetricMatrixPtr contactMap = supervisionObject.dynamicCast<ScoreSymmetricMatrix>();
  jassert(contactMap);
  if (!contactMap || !contactMap->hasScore(firstPosition, secondPosition))
    return ObjectPtr();
  return new Label(BinaryClassificationDictionary::getInstance(), contactMap->getScore(firstPosition, secondPosition) > 0.5 ? 1 : 0);
}

void ContactMapScoresInference::setSubOutput(ObjectPtr output, size_t firstPosition, size_t secondPosition, ObjectPtr subOutput) const
{
  ScoreSymmetricMatrixPtr contactMap = output.dynamicCast<ScoreSymmetricMatrix>();
  jassert(contactMap);
  double contactScore = 0.5;
  LabelPtr prediction = subOutput.dynamicCast<Label>();
  if (prediction)
  {
    jassert(prediction->getDictionary() == BinaryClassificationDictionary::getInstance());
    contactScore = prediction->getIndex() == 1 ? prediction->getScore() : -prediction->getScore();
  }
  else
  {
    ScalarPtr scalar = subOutput.dynamicCast<Scalar>();
    if (scalar)
      contactScore = scalar->getValue();
    else
      return;
  }
  contactMap->setScore(firstPosition, secondPosition, contactScore);
}

/*
** ContactMapScoresToProbabilitiesInference
*/
ContactMapScoresToProbabilitiesInference::ContactMapScoresToProbabilitiesInference(const String& name, const String& targetName)
  : Inference(name), ProteinTargetInferenceHelper(targetName), threshold(0.0)
  {}

Variable ContactMapScoresToProbabilitiesInference::run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  ObjectPairPtr inputProteinAndContactMap = input.dynamicCast<ObjectPair>();
  jassert(inputProteinAndContactMap);
  ProteinPtr inputProtein = inputProteinAndContactMap->getFirst().dynamicCast<Protein>();
  ScoreSymmetricMatrixPtr scoresContactMap = inputProteinAndContactMap->getSecond().dynamicCast<ScoreSymmetricMatrix>();
  jassert(inputProtein);
  if (!scoresContactMap)
    return ObjectPtr();

  ScoreSymmetricMatrixPtr res = inputProtein->createEmptyObject(targetName);
  size_t n = scoresContactMap->getDimension();
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i; j < n; ++j)
      if (scoresContactMap->hasScore(i, j))
      {
        static const double temperature = 1.0;
        double score = scoresContactMap->getScore(i, j) - threshold;
        double probability = 1.0 / (1.0 + exp(-score * temperature));
        //std::cout << scoresContactMap->getScore(i, j)  << " threshold = " << threshold << " ==> " << probability << std::endl;
        res->setScore(i, j, probability);
      }
  return res;
}
  
/*
** ContactMapScoresToProbabilitiesInferenceBatchLearner
*/
class ContactMapScoresToProbabilitiesInferenceBatchLearner : public Inference
{
protected:
  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ObjectPairPtr inferenceAndTrainingData = input.dynamicCast<ObjectPair>();
    jassert(inferenceAndTrainingData);
    InferencePtr inference = inferenceAndTrainingData->getFirst().dynamicCast<Inference>();
    ObjectContainerPtr trainingData = inferenceAndTrainingData->getSecond().dynamicCast<ObjectContainer>();
    jassert(inference && trainingData);
    returnCode = train(context, inference, trainingData);
    return ObjectPtr();
  }

  ReturnCode train(InferenceContextPtr context, InferencePtr inf, ObjectContainerPtr trainingData)
  {
    std::cout << "LOOKING FOR BEST THRESHOLD ..... " << std::endl;
    ContactMapScoresToProbabilitiesInferencePtr inference = inf.dynamicCast<ContactMapScoresToProbabilitiesInference>();
    jassert(inference);
    ROCAnalyse roc;
    fillROCAnalyse(roc, inference->getSupervisionName(), trainingData);
    double bestThreshold = findBestThreshold(roc);
    std::cout << "BEST THRESHOLD: " << bestThreshold << std::endl; 
    inference->setThreshold(bestThreshold);
    return finishedReturnCode;
  }
  
  double findBestThreshold(ROCAnalyse& roc)
  {
    double f1, precision, recall;
    //bestThreshold = roc.findThresholdMaximisingRecallGivenPrecision(0.5, recall);
    return roc.findThresholdMaximisingF1(f1, precision, recall);
  }
  
  void fillROCAnalyse(ROCAnalyse& roc, const String& supervisionObjectName, ObjectContainerPtr trainingData)
  {
    for (size_t exampleNumber = 0; exampleNumber < trainingData->size(); ++exampleNumber)
    {
      // get input and supervision
      ObjectPairPtr inputAndSupervision = trainingData->getAndCast<ObjectPair>(exampleNumber);
      jassert(inputAndSupervision);
      ObjectPtr supervision = inputAndSupervision->getSecond();
      if (!supervision)
        continue;
      
      // input: get predicted scores contact map
      ObjectPairPtr inputProteinAndContactMap = inputAndSupervision->getFirst().dynamicCast<ObjectPair>();
      jassert(inputProteinAndContactMap);
      ScoreSymmetricMatrixPtr scoresContactMap = inputProteinAndContactMap->getSecond().dynamicCast<ScoreSymmetricMatrix>();
      if (!scoresContactMap)
        continue;
      
      // supervision: get correct protein and correct contact map
      ProteinPtr correctProtein = supervision.dynamicCast<Protein>();
      jassert(correctProtein);
      ScoreSymmetricMatrixPtr supervisionContactMap = correctProtein->getObject(supervisionObjectName);
      jassert(supervisionContactMap && supervisionContactMap->getDimension() == scoresContactMap->getDimension());
    
      // fill ROC
      size_t n = scoresContactMap->getDimension();
      for (size_t i = 0; i < n; ++i)
        for (size_t j = i; j < n; ++j)
          if (supervisionContactMap->hasScore(i, j) && scoresContactMap->hasScore(i, j))
          {
            bool isPositive = supervisionContactMap->getScore(i, j) > 0.5;
            roc.addPrediction(scoresContactMap->getScore(i, j), isPositive);
          }
    }
  }
};

/*
** ProteinContactMapInference
*/
ProteinContactMapInference::ProteinContactMapInference(const String& name, InferencePtr scoreInference, ProteinResiduePairFeaturesPtr scoreFeatures, const String& targetName)
  : VectorSequentialInference(name), ProteinTargetInferenceHelper(targetName)
{
  setBatchLearner(sequentialInferenceLearner());
  
  InferencePtr scoresInference(new ContactMapScoresInference(name, scoreInference, scoreFeatures, targetName));
  scoresInference->setBatchLearner(simulationInferenceLearner());
  appendInference(scoresInference);
  
  InferencePtr probabilitiesInference(new ContactMapScoresToProbabilitiesInference(name + T("toProb"), targetName));
  probabilitiesInference->setBatchLearner(new ContactMapScoresToProbabilitiesInferenceBatchLearner());
  appendInference(probabilitiesInference);
}

SequentialInferenceStatePtr ProteinContactMapInference::prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  jassert(subInferences.size() == 2);
  SequentialInferenceStatePtr state = new SequentialInferenceState(input, supervision);
  state->setSubInference(subInferences.get(0), input, supervision);
  return state;
}

bool ProteinContactMapInference::updateInference(InferenceContextPtr context, SequentialInferenceStatePtr state, ReturnCode& returnCode)
{
  if (state->getStepNumber() == 0)
  {
    state->setSubInference(subInferences.get(1), ObjectPtr(new ObjectPair(state->getInput(), state->getSubOutput())), state->getSupervision());
    return true;
  }   
  return false;
}
