/*-----------------------------------------.---------------------------------.
| Filename: ProteinContactMapInference.cpp | ProteinObject Contact Map Inference   |
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

void ContactMapScoresInference::computeSubStepIndices(ProteinObjectPtr protein, std::vector< std::pair<size_t, size_t> >& res) const
{
  size_t n = protein->getLength();
  res.reserve(n * (n - 5) / 2);
  for (size_t i = 0; i < n; ++i)
    for (size_t j = i + 6; j < n; ++j)
      res.push_back(std::make_pair(i, j));
}

Variable ContactMapScoresInference::getSubSupervision(ObjectPtr supervisionObject, size_t firstPosition, size_t secondPosition) const
{
  if (!supervisionObject)
    return Variable();
  ScoreSymmetricMatrixPtr contactMap = supervisionObject.dynamicCast<ScoreSymmetricMatrix>();
  jassert(contactMap);
  if (!contactMap || !contactMap->hasScore(firstPosition, secondPosition))
    return Variable();
  return contactMap->getScore(firstPosition, secondPosition) > 0.5;
}

void ContactMapScoresInference::setSubOutput(ObjectPtr output, size_t firstPosition, size_t secondPosition, const Variable& subOutput) const
{
  ScoreSymmetricMatrixPtr contactMap = output.dynamicCast<ScoreSymmetricMatrix>();
  jassert(contactMap);
  if (subOutput)
    contactMap->setScore(firstPosition, secondPosition, subOutput.getDouble());
}

/*
** ContactMapScoresToProbabilitiesInference
*/
ContactMapScoresToProbabilitiesInference::ContactMapScoresToProbabilitiesInference(const String& name, const String& targetName)
  : Inference(name), ProteinTargetInferenceHelper(targetName), threshold(0.0)
  {}

// Input: (input protein, predicted CM scores) pair
Variable ContactMapScoresToProbabilitiesInference::run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  jassert(input.size() == 2);
  ProteinObjectPtr inputProtein = input[0].getObjectAndCast<ProteinObject>();
  ScoreSymmetricMatrixPtr scoresContactMap = input[1].getObjectAndCast<ScoreSymmetricMatrix>();
  jassert(inputProtein);
  if (!scoresContactMap)
    return Variable();

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
    jassert(input.getType()->inheritsFrom(pairType()));

    InferencePtr inference = input[0].getObject().dynamicCast<Inference>();
    ContainerPtr trainingData = input[1].getObjectAndCast<Container>();
    jassert(inference && trainingData);
    returnCode = train(context, inference, trainingData);
    return ObjectPtr();
  }

  ReturnCode train(InferenceContextPtr context, InferencePtr inf, ContainerPtr trainingData)
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
  
  void fillROCAnalyse(ROCAnalyse& roc, const String& supervisionObjectName, ContainerPtr trainingData)
  {
    for (size_t exampleNumber = 0; exampleNumber < trainingData->getNumVariables(); ++exampleNumber)
    {
      // get input and supervision
      Variable inputAndSupervision = trainingData->getVariable(exampleNumber);
      jassert(inputAndSupervision);
      ProteinObjectPtr correctProtein = inputAndSupervision[1].getObjectAndCast<ProteinObject>();
      if (!correctProtein)
        continue;
      
      // input: get predicted scores contact map
      Variable inputProteinAndContactMap = inputAndSupervision[0];
      jassert(inputProteinAndContactMap.size() == 2);
      ScoreSymmetricMatrixPtr scoresContactMap = inputProteinAndContactMap[1].getObjectAndCast<ScoreSymmetricMatrix>();
      if (!scoresContactMap)
        continue;
      
      // supervision: get correct protein and correct contact map
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
  InferencePtr scoresInference(new ContactMapScoresInference(name, scoreInference, scoreFeatures, targetName));
  //scoresInference->setBatchLearner(simulationInferenceLearner());
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
    state->setSubInference(subInferences.get(1), Variable::pair(state->getInput(), state->getSubOutput()), state->getSupervision());
    return true;
  }   
  return false;
}
