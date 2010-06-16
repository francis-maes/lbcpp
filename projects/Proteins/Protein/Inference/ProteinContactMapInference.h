/*-----------------------------------------.---------------------------------.
| Filename: ProteinContactMapInferenceStep.h| Protein Contact Map Inference  |
| Author  : Francis Maes                   |                                 |
| Started : 28/04/2010 11:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_
# define LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_

# include "Protein2DTargetInference.h"
# include <algorithm>

namespace lbcpp
{

class ContactMapScoresInference : public Protein2DTargetInference
{
public:
  ContactMapScoresInference(const String& name, InferencePtr scoreInference, ProteinResiduePairFeaturesPtr features, const String& targetName)
    : Protein2DTargetInference(name, scoreInference, features, targetName)
    {}

  ContactMapScoresInference() {}

  virtual void computeSubStepIndices(ProteinPtr protein, std::vector< std::pair<size_t, size_t> >& res) const
  {
    size_t n = protein->getLength();
    res.reserve(n * (n - 5) / 2);
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i + 6; j < n; ++j)
        res.push_back(std::make_pair(i, j));
  }

  virtual ObjectPtr getSubSupervision(ObjectPtr supervisionObject, size_t firstPosition, size_t secondPosition) const
  {
    if (!supervisionObject)
      return ObjectPtr();
    ScoreSymmetricMatrixPtr contactMap = supervisionObject.dynamicCast<ScoreSymmetricMatrix>();
    jassert(contactMap);
    if (!contactMap || !contactMap->hasScore(firstPosition, secondPosition))
      return ObjectPtr();
    return new Label(BinaryClassificationDictionary::getInstance(), contactMap->getScore(firstPosition, secondPosition) > 0.5 ? 1 : 0);
  }

  virtual void setSubOutput(ObjectPtr output, size_t firstPosition, size_t secondPosition, ObjectPtr subOutput) const
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
};

class ContactMapScoresToProbabilitiesInference : public DecoratorInference, public ProteinTargetInferenceHelper
{
public:
  ContactMapScoresToProbabilitiesInference(const String& name, InferencePtr thresholdRegression, ProteinGlobalFeaturesPtr thresholdFeatures, const String& targetName)
    : DecoratorInference(name, thresholdRegression), ProteinTargetInferenceHelper(targetName), thresholdFeatures(thresholdFeatures)
    {}
  ContactMapScoresToProbabilitiesInference() {}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ObjectPairPtr inputProteinAndContactMap = input.dynamicCast<ObjectPair>();
    jassert(inputProteinAndContactMap);
    ProteinPtr inputProtein = inputProteinAndContactMap->getFirst().dynamicCast<Protein>();
    ScoreSymmetricMatrixPtr scoresContactMap = inputProteinAndContactMap->getSecond().dynamicCast<ScoreSymmetricMatrix>();
    jassert(inputProtein);
    if (!scoresContactMap)
      return ObjectPtr();

    ScalarPtr subSupervision;

    ProteinPtr correctProtein = supervision.dynamicCast<Protein>();
    jassert(!supervision || correctProtein);
    double bestThreshold = 0.0;
    if (correctProtein)
    {
      ScoreSymmetricMatrixPtr supervisionContactMap = correctProtein->getObject(supervisionName);
      jassert(supervisionContactMap && supervisionContactMap->getDimension() == scoresContactMap->getDimension());
      ROCAnalyse roc;
      size_t n = scoresContactMap->getDimension();
      bool atLeastOneContact = false;
      for (size_t i = 0; i < n; ++i)
        for (size_t j = i; j < n; ++j)
          if (supervisionContactMap->hasScore(i, j) && scoresContactMap->hasScore(i, j))
          {
            bool isPositive = supervisionContactMap->getScore(i, j) > 0.5;
            atLeastOneContact |= isPositive;
            roc.addPrediction(scoresContactMap->getScore(i, j), isPositive);
          }

      if (atLeastOneContact)
      {
        double recall;
        bestThreshold = roc.findThresholdMaximisingRecallGivenPrecision(0.5, recall);
        if (recall)
        {
          //std::cout << "Best-Threshold: " << bestThreshold << " => Recall = " << String(recall * 100, 2) << "%" << std::endl;
          subSupervision = new Scalar(bestThreshold);
        }
      }
    }

    ScalarPtr predictedThreshold = DecoratorInference::run(context, thresholdFeatures->compute(inputProtein), subSupervision, returnCode);
    double threshold = 0.0;
    if (predictedThreshold)
    {
      threshold = predictedThreshold->getValue();
      //std::cout << "Predicted threshold: " << predictedThreshold->getValue() << " best threshold: " << lbcpp::toString(subSupervision) << std::endl;
    }
    ScoreSymmetricMatrixPtr res = inputProtein->createEmptyObject(targetName);
    size_t n = scoresContactMap->getDimension();
    for (size_t i = 0; i < n; ++i)
      for (size_t j = i; j < n; ++j)
        if (scoresContactMap->hasScore(i, j))
        {
          static const double temperature = 1.0;
          double score = scoresContactMap->getScore(i, j) - threshold;
          double probability = 1.0 / (1.0 + exp(-score * temperature));
          res->setScore(i, j, probability);
        }
    return res;
  }

private:
  ProteinGlobalFeaturesPtr thresholdFeatures;
};

class ProteinContactMapInference : public VectorSequentialInference, public ProteinTargetInferenceHelper
{
public:
  ProteinContactMapInference(const String& name, InferencePtr scoreInference, ProteinResiduePairFeaturesPtr scoreFeatures, 
                                                 InferencePtr thresholdInference, ProteinGlobalFeaturesPtr thresholdFeatures, const String& targetName)
    : VectorSequentialInference(name), ProteinTargetInferenceHelper(targetName)
  {
    appendInference(new ContactMapScoresInference(name, scoreInference, scoreFeatures, targetName));
    appendInference(new ContactMapScoresToProbabilitiesInference(name + T("toProb"), thresholdInference, thresholdFeatures, targetName));
  }
  ProteinContactMapInference() {}

  virtual ObjectPairPtr prepareSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const
  {
    if (state->getCurrentStepNumber() == 0)
      return new ObjectPair(state->getInput(), state->getSupervision());
    else
      // the second sub-inference receives both the protein and the predicted contactmap scores
      return new ObjectPair(new ObjectPair(state->getInput(), state->getCurrentObject()), state->getSupervision());
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_
