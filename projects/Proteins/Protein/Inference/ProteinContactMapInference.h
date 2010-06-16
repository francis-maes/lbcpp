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

class ProteinContactMapInference : public Protein2DTargetInference
{
public:
  ProteinContactMapInference(const String& name, InferencePtr contactInference, ProteinResiduePairFeaturesPtr features, const String& targetName)
    : Protein2DTargetInference(name, contactInference, features, targetName)
    {}

  ProteinContactMapInference() {}

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
      contactScore = 1.0 / (1.0 + exp(-contactScore));
    }
    else
    {
      ScalarPtr scalar = subOutput.dynamicCast<Scalar>();
      if (scalar)
        contactScore = scalar->getValue();
      else
        return;
    }
    jassert(contactScore >= 0.0 && contactScore <= 1.0);
    contactMap->setScore(firstPosition, secondPosition, contactScore);
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_
